//
//  BluseBasic_Flashstore.c
//  BlueBasic
//
//  Created by tim on 9/7/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#include "os.h"

extern __data unsigned char* heap;
extern __data unsigned char* sp;

#define FLASHSTORE_WORDS(LEN)     ((LEN) >> 2)

#ifdef SIMULATE_FLASH
unsigned char __store[FLASHSTORE_LEN];
#define FLASHSTORE_CPU_BASEADDR (__store)
#define FLASHSTORE_DMA_BASEADDR (0)
#endif

static const unsigned char* flashstore = (unsigned char*)FLASHSTORE_CPU_BASEADDR;
#define FLASHSTORE_FADDR(ADDR)  ((((unsigned char*)(ADDR) - FLASHSTORE_CPU_BASEADDR) + FLASHSTORE_DMA_BASEADDR) >> 2)
#define FLASHSTORE_FPAGE(ADDR)  (FLASHSTORE_FADDR(ADDR) >> 9)

static unsigned short** lineindexstart;
static unsigned short** lineindexend;
static struct
{
  unsigned short free;
  unsigned short waste;
} orderedpages[FLASHSTORE_NRPAGES];

#define FLASHSTORE_PAGEBASE(IDX)  &flashstore[FLASHSTORE_PAGESIZE * (IDX)]
#define FLASHSTORE_PADDEDSIZE(SZ) (((SZ) + 3) & -4)


//
// Flash page structure:
//  <age:4><data:FLASHSTORE_PAGESIZE-4>
//  Each page is given an age, starting at 1, as they are used. An age of 0xFFFFFFFF means the page is empty.
//  The ages are used to reconstruct the program data by keeping the pages use ordered correctly.
//
#ifdef TARGET_CC254X
typedef unsigned long flashpage_age;
#else
typedef unsigned int flashpage_age;
#endif
static flashpage_age lastage = 1;

//
// Flash item structure:
//  <id:2><len:1><data:len>
//  Id's between 1 and 0xFFFD are valid program lines. Id==0 is an invalidated item, Id=0xFFFE is special (undefined)
//  and Id=0xFFFF is free space to the end of the page.
//

static void flashstore_invalidate(unsigned short* mem);

//
// Heapsort
//  Modified from: http://www.algorithmist.com/index.php/Heap_sort.c
//
static void siftdown(short root, short bottom)
{
  unsigned short** lines = lineindexstart;
  for (;;)
  {
    short max = root * 2 + 1;
    
    if (max > bottom)
    {
      return;
    }
    if (max < bottom)
    {
      short other = max + 1;
      if (*lines[other] > *lines[max])
      {
        max = other;
      }
    }
    if (*lines[root] >= *lines[max])
    {
      return;
    }
    
    unsigned short* temp = lines[root];
    lines[root] = lines[max];
    lines[max] = temp;

    root = max;
  }
}

static void flashpage_heapsort(void)
{
  short i;
  unsigned short** lines = lineindexstart;
  unsigned short count = lineindexend - lineindexstart;

  for (i = count / 2; i >= 0; i--)
  {
    siftdown(i, count - 1);
  }
  for (i = count - 1; i >= 0; i--)
  {
    unsigned short* temp = lines[0];
    lines[0] = lines[i];
    lines[i] = temp;
    siftdown(0, i - 1);
  }
}


//
// Initialize the flashstore.
//  Rebuild the program store from the flash store.
//  Called when the interpreter powers up.
//
unsigned char** flashstore_init(unsigned char** startmem)
{
  lineindexstart = (unsigned short**)startmem;
  lineindexend = lineindexstart;

  OS_flashstore_init();

  unsigned char ordered = 0;
  const unsigned char* page;
  for (page = flashstore; page < &flashstore[FLASHSTORE_LEN]; page += FLASHSTORE_PAGESIZE)
  {
    orderedpages[ordered].waste = 0;
    if (*(flashpage_age*)page > lastage)
    {
      lastage = *(flashpage_age*)page;
    }

    // Analyse page
    const unsigned char* ptr;
    for (ptr = page + sizeof(flashpage_age); ptr < page + FLASHSTORE_PAGESIZE; )
    {
      unsigned short id = *(unsigned short*)ptr;
      if (id == FLASHID_FREE)
      {
        break;
      }
      else if (id == FLASHID_INVALID)
      {
        orderedpages[ordered].waste += FLASHSTORE_PADDEDSIZE(ptr[sizeof(unsigned short)]);
      }
      else if (id != FLASHID_SPECIAL)
      {
        // Valid program line - record entry (sort later)
        *lineindexend++ = (unsigned short*)ptr;
      }
      ptr += FLASHSTORE_PADDEDSIZE(ptr[sizeof(unsigned short)]);
    }
  
    orderedpages[ordered].free = FLASHSTORE_PAGESIZE - (ptr - page);
    ordered++;
  }

  // We now have a set of program lines, indexed from "startmem" to "mem" which we need to sort
  flashpage_heapsort();
  
  return (unsigned char**)lineindexend;
}

//
// Find the closest, lower or equal, ID
//
unsigned short** flashstore_findclosest(unsigned short id)
{
  unsigned short** lines = lineindexstart;
  unsigned short min = 0;
  unsigned short max = lineindexend - lines;
  if (max > 0)
  {
    max -= 1;
    while (min < max)
    {
      unsigned short mid = min + (max - min) / 2;
      if (*lineindexstart[mid] < id)
      {
        min = mid + 1;
      }
      else
      {
        max = mid;
      }
    }
  }
  return lines + min;
}

//
// Find space for the new line in the youngest page
//
static signed char flashstore_findspace(unsigned char len)
{
  unsigned char pg;
  unsigned char spg = 0;
  flashpage_age age = 0xFFFFFFFF;
  for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
  {
    flashpage_age cage = *(flashpage_age*)FLASHSTORE_PAGEBASE(pg);
    if (cage < age && orderedpages[pg].free >= len)
    {
      spg = pg;
      age = cage;
    }
  }
  return age == 0xFFFFFFFF ? -1 : spg;
}

//
// Add a new line to the flash store, returning the total number of lines.
//
unsigned char** flashstore_addline(unsigned char* line)
{
  unsigned short id = *(unsigned short*)line;
  unsigned short** oldlineptr = flashstore_findclosest(id);
  unsigned char found = 0;
  if (oldlineptr < lineindexend && **oldlineptr == id)
  {
    found = 1;
  }

  // Find space for the new line in the youngest page
  unsigned char len = FLASHSTORE_PADDEDSIZE(line[sizeof(unsigned short)]);
  signed char pg = flashstore_findspace(len);
  if (pg != -1)
  {
    unsigned short* mem = (unsigned short*)(FLASHSTORE_PAGEBASE(pg) + FLASHSTORE_PAGESIZE - orderedpages[pg].free);
    OS_flashstore_write(FLASHSTORE_FADDR(mem), line, FLASHSTORE_WORDS(len));
    orderedpages[pg].free -= len;
    // If there was an old version, invalidate it
    if (found)
    {
      // If there was an old version, invalidate it
      flashstore_invalidate(*oldlineptr);

      // Insert new line into index
      *oldlineptr = mem;
    }
    else
    {
      // Insert new line
      unsigned short len = sizeof(unsigned short*) * (lineindexend - oldlineptr);
      if (len == 0)
      {
        oldlineptr[0] = mem;
      }
      else
      {
        OS_rmemcpy(oldlineptr + 1, oldlineptr, len);
        if (**oldlineptr > id)
        {
          oldlineptr[0] = mem;
        }
        else
        {
          oldlineptr[1] = mem;
        }
      }
      lineindexend++;
    }
    return (unsigned char**)lineindexend;
  }
  else
  {
    // No space
    return NULL;
  }
}

unsigned char flashstore_addspecial(unsigned char* item)
{
  *(unsigned short*)item = FLASHID_SPECIAL;
  unsigned char len = FLASHSTORE_PADDEDSIZE(item[sizeof(unsigned short)]);
  // Find space for the new line in the youngest page
  signed char pg = flashstore_findspace(len);
  if (pg != -1)
  {
    unsigned short* mem = (unsigned short*)(FLASHSTORE_PAGEBASE(pg) + FLASHSTORE_PAGESIZE - orderedpages[pg].free);
    OS_flashstore_write(FLASHSTORE_FADDR(mem), item, FLASHSTORE_WORDS(len));
    orderedpages[pg].free -= len;
    return 1;
  }
  else
  {
    return 0;
  }
}

unsigned char flashstore_deletespecial(unsigned long specialid)
{
  unsigned char* ptr = flashstore_findspecial(specialid);
  if (ptr)
  {
    flashstore_invalidate((unsigned short*)ptr);
    return 1;
  }
  return 0;
}

unsigned char* flashstore_findspecial(unsigned long specialid)
{
  const unsigned char* page;
  for (page = flashstore; page < &flashstore[FLASHSTORE_LEN]; page += FLASHSTORE_PAGESIZE)
  {
    const unsigned char* ptr;
    for (ptr = page + sizeof(flashpage_age); ptr < page + FLASHSTORE_PAGESIZE; ptr += FLASHSTORE_PADDEDSIZE(ptr[sizeof(unsigned short)]))
    {
      unsigned short id = *(unsigned short*)ptr;
      if (id == FLASHID_FREE)
      {
        break;
      }
      else if (id == FLASHID_SPECIAL && *(unsigned long*)(ptr + FLASHSPECIAL_ITEM_ID) == specialid)
      {
        return (unsigned char*)ptr;
      }
    }
  }
  return NULL;
}

//
// Remove the line from the flash store.
//
unsigned char** flashstore_deleteline(unsigned short id)
{
  unsigned short** oldlineptr = flashstore_findclosest(id);
  if (*oldlineptr != NULL && **oldlineptr == id)
  {
    lineindexend--;
    flashstore_invalidate(*oldlineptr);
    OS_memcpy(oldlineptr, oldlineptr + 1, sizeof(unsigned short*) * (lineindexend - oldlineptr));
  }
  return (unsigned char**)lineindexend;
}

//
// Delete everything from the store.
//
unsigned char** flashstore_deleteall(void)
{
  unsigned char pg;
  for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
  {
    if (orderedpages[pg].free != FLASHSTORE_PAGESIZE - sizeof(flashpage_age))
    {
      const unsigned char* base = FLASHSTORE_PAGEBASE(pg);
      OS_flashstore_erase(FLASHSTORE_FPAGE(base));
      OS_flashstore_write(FLASHSTORE_FADDR(base), (unsigned char*)&lastage, FLASHSTORE_WORDS(sizeof(lastage)));
      lastage++;
      orderedpages[pg].waste = 0;
      orderedpages[pg].free = FLASHSTORE_PAGESIZE - sizeof(flashpage_age);
    }
  }

  lineindexend = lineindexstart;
  return (unsigned char**)lineindexend;
}

//
// How much space is free?
//
unsigned int flashstore_freemem(void)
{
  unsigned int free = 0;
  unsigned char pg;
  for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
  {
    free += orderedpages[pg].free;
  }
  return free;
}

void flashstore_compact(unsigned char len, unsigned char* tempmemstart, unsigned char* tempmemend)
{
  // Need at least FLASHSTORE_PAGESIZE
  if (tempmemend - tempmemstart < FLASHSTORE_PAGESIZE)
  {
    return;
  }

  // Find the lowest age page which this will fit in.
  unsigned char pg;
  unsigned char selected = 0;
  flashpage_age age = 0xFFFFFFFF;
  for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
  {
    flashpage_age cage = *(flashpage_age*)FLASHSTORE_PAGEBASE(pg);
    if (cage < age && orderedpages[pg].waste + orderedpages[pg].free >= len)
    {
      selected = pg;
      age = cage;
    }
  }
  if (age != 0xFFFFFFFF)
  {
    // Found enough space for the line, compact the page
    
    // Copy the page into RAM
    unsigned char* ram = tempmemstart;
    unsigned char* flash = (unsigned char*)FLASHSTORE_PAGEBASE(selected);
    OS_memcpy(ram, flash, FLASHSTORE_PAGESIZE);

    // Erase the page
    OS_flashstore_erase(FLASHSTORE_FPAGE(flash));
    OS_flashstore_write(FLASHSTORE_FADDR(flash), (unsigned char*)&lastage, FLASHSTORE_WORDS(sizeof(lastage)));
    lastage++;
    orderedpages[selected].waste = 0;
    orderedpages[selected].free = FLASHSTORE_PAGESIZE - sizeof(flashpage_age);

    // Copy the old lines back in. More efficient ways to do this, but okay for the moment
    unsigned char* ptr;
    for (ptr = ram + sizeof(flashpage_age); ptr < ram + FLASHSTORE_PAGESIZE; )
    {
      unsigned short id = *(unsigned short*)ptr;
      unsigned char len = FLASHSTORE_PADDEDSIZE(ptr[sizeof(unsigned short)]);
      if (id == FLASHID_FREE)
      {
        break;
      }
      else if (id != FLASHID_INVALID)
      {
        OS_flashstore_write(FLASHSTORE_FADDR(ptr - ram + flash), ptr, FLASHSTORE_WORDS(len));
        orderedpages[selected].free -= len;
      }
      ptr += len;
    }
  
    // We corrupted memory, so we need to reinitialize
    flashstore_init((unsigned char**)lineindexstart);
  }
}

//
// Invalidate the line entry at the given address.
//
static void flashstore_invalidate(unsigned short* mem)
{
  struct
  {
    unsigned short invalid;
    unsigned char len;
    unsigned char padding;
  } invalid;
  OS_memcpy(&invalid, mem, sizeof(invalid));
  invalid.invalid = FLASHID_INVALID;

  OS_flashstore_write(FLASHSTORE_FADDR(mem), (unsigned char*)&invalid, FLASHSTORE_WORDS(sizeof(invalid)));

  orderedpages[((unsigned char*)mem - flashstore) / FLASHSTORE_PAGESIZE].waste += invalid.len;
}

//
// Support the OSAL flash API
//

unsigned char osal_snv_read(unsigned char id, unsigned char len, void *pBuf)
{
  unsigned char* mem = flashstore_findspecial(FLASHSPECIAL_SNV + id);
  if (mem && mem[FLASHSPECIAL_DATA_LEN] == len)
  {
    OS_memcpy(pBuf, mem + FLASHSPECIAL_DATA_OFFSET, len);
    return 1;
  }
  else
  {
    return 0;
  }
}

unsigned char osal_snv_write(unsigned char id, unsigned char len, void *pBuf)
{      
  if (heap + len + FLASHSPECIAL_DATA_OFFSET > sp)
  {
    return 0;
  }
  else
  {
    unsigned char* item = heap;
    heap += len + FLASHSPECIAL_DATA_OFFSET;

    *(unsigned long*)&item[FLASHSPECIAL_ITEM_ID] = FLASHSPECIAL_SNV + id;
    item[FLASHSPECIAL_DATA_LEN] = len;
    OS_memcpy(item + FLASHSPECIAL_DATA_OFFSET, pBuf, len);
    unsigned char r = flashstore_addspecial(item);
    
    heap = item;
    return r;
  }
}

unsigned char osal_snv_compact(unsigned char threshold)
{
  return 0;
}
