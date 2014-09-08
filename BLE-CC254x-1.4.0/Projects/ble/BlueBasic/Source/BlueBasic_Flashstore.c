//
//  BluseBasic_Flashstore.c
//  BlueBasic
//
//  Created by tim on 9/7/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#include "os.h"

#define FLASHSTORE_NRPAGES    4
#define FLASHSTORE_PAGESIZE   2048
#define FLASHSTORE_WRITESIZE  4

#define FLASHSTORE_LEN        (FLASHSTORE_NRPAGES * FLASHSTORE_PAGESIZE)

#ifdef SIMULATE_FLASH
static unsigned char __store[FLASHSTORE_LEN];
static unsigned char* flashstore = __store;
#endif
static unsigned short** lineindexstart;
static unsigned short** lineindexend;
static struct
{
  unsigned short free;
  unsigned short waste;
} orderedpages[FLASHSTORE_NRPAGES];
#define FLASHSTORE_PAGEBASE(IDX)  &flashstore[FLASHSTORE_PAGESIZE * (IDX)]

//
// Flash page structure:
//  <age:4><data:FLASHSTORE_PAGESIZE-4>
//  Each page is given an age, starting at 1, as they are used. An age of 0xFFFFFFFF means the page is empty.
//  The ages are used to reconstruct the program data by keeping the pages use ordered correctly.
//
#ifdef TARGET_CC254X
typedef long int flashpage_age;
#else
typedef unsigned int flashpage_age;
#endif
static flashpage_age lastage;

//
// Flash item structure:
//  <id:2><len:1><data:len>
//  Id's between 1 and 0xFFFD are valid program lines. Id==0 is an invalidated item, Id=0xFFFE is special (undefined)
//  and Id=0xFFFF is free space to the end of the page.
//
enum
{
  FLASHID_INVALID = 0x0000,
  FLASHID_SPECIAL = 0xFFFE,
  FLASHID_FREE    = 0xFFFF,
};

static unsigned char flashstore_compact(unsigned char len);
static void flashstore_write(unsigned char* mem, unsigned char* value, unsigned char size);
static void flashstore_erase(unsigned char* mem);
#define FLASHSTORE_WRITEID(MEM, ID) do { unsigned short id = (ID); flashstore_write((unsigned char*)(MEM), (unsigned char*)&id, sizeof(id)); } while(0)


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
// Initializd the flashstore.
//  Rebuild the program store from the flash store.
//  Called when the interpreter powers up.
//
unsigned char** flashstore_init(unsigned char** startmem)
{
  lineindexstart = (unsigned short**)startmem;
  lineindexend = lineindexstart;
  lastage = 1;
#ifdef SIMULATE_FLASH
  FILE* fp = fopen("/tmp/flashstore", "r");
  if (fp)
  {
    fread(__store, FLASHSTORE_LEN, sizeof(char), fp);
    fclose(fp);
  }
  else
  {
    unsigned char* ptr;
    memset(__store, 0xFF, FLASHSTORE_LEN);
    for (ptr = flashstore; ptr < &flashstore[FLASHSTORE_LEN]; ptr += FLASHSTORE_PAGESIZE)
    {
      *(flashpage_age*)ptr = lastage++;
    }

  }
#endif
  unsigned char ordered = 0;
  unsigned char* page;
  for (page = flashstore; page < &flashstore[FLASHSTORE_LEN]; page += FLASHSTORE_PAGESIZE)
  {
    orderedpages[ordered].waste = 0;
    if (*(flashpage_age*)page > lastage)
    {
      lastage = *(flashpage_age*)page;
    }

    // Analyse page
    unsigned char* ptr;
    for (ptr = page + sizeof(flashpage_age); ptr < page + FLASHSTORE_PAGESIZE; )
    {
      unsigned short id = *(unsigned short*)ptr;
      if (id == FLASHID_FREE)
      {
        break;
      }
      else if (id == FLASHID_INVALID)
      {
        orderedpages[ordered].waste += ptr[sizeof(unsigned short)];
      }
      else if (id != FLASHID_SPECIAL)
      {
        // Valid program line - record entry (sort later)
        *lineindexend++ = (unsigned short*)ptr;
      }
      ptr += ptr[sizeof(unsigned short)];
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
// Add a new line to the flash store, returning the total number of lines.
//
unsigned char** flashstore_addline(unsigned char* line, unsigned char len)
{
  unsigned short id = *(unsigned short*)line;
  unsigned short** oldlineptr = flashstore_findclosest(id);
  unsigned char found = 0;
  if (oldlineptr < lineindexend && **oldlineptr == id)
  {
    found = 1;
  }
  
  for (;;)
  {
    // Find space for the new line
    unsigned char pg;
    for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
    {
      if (orderedpages[pg].free >= len)
      {
        unsigned short* mem = (unsigned short*)(FLASHSTORE_PAGEBASE(pg) + FLASHSTORE_PAGESIZE - orderedpages[pg].free);
        flashstore_write((unsigned char*)mem, line, len);
        orderedpages[pg].free -= len;
        // If there was an old version, invalidate it
        if (found)
        {
          // If there was an old version, invalidate it
          FLASHSTORE_WRITEID(*oldlineptr, FLASHID_INVALID);
          // Add the waste to its page
          orderedpages[((unsigned char*)*oldlineptr - flashstore) / FLASHSTORE_PAGESIZE].waste += ((unsigned char*)*oldlineptr)[sizeof(unsigned short)];
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
    }
    // No space - we need to compact
    if (!flashstore_compact(len))
    {
      // Failed to find enough space
      break;
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
    FLASHSTORE_WRITEID(*oldlineptr, FLASHID_INVALID);
    orderedpages[((unsigned char*)*oldlineptr - flashstore) / FLASHSTORE_PAGESIZE].waste += ((unsigned char*)*oldlineptr)[sizeof(unsigned short)];
    OS_memcpy(oldlineptr, oldlineptr + 1, sizeof(unsigned short*) * (lineindexend - oldlineptr));
  }
  return (unsigned char**)lineindexend;
}

//
// Delete everything from the store.
//
extern unsigned char** flashstore_deleteall(void)
{
  unsigned char pg;
  for (pg = 0; pg < FLASHSTORE_NRPAGES; pg++)
  {
    if (orderedpages[pg].free != FLASHSTORE_PAGESIZE - sizeof(flashpage_age))
    {
      unsigned char* base = FLASHSTORE_PAGEBASE(pg);
      flashstore_erase(base);
      flashstore_write(base, (unsigned char*)&lastage, sizeof(lastage));
      lastage++;
      orderedpages[pg].waste = 0;
      orderedpages[pg].free = FLASHSTORE_PAGESIZE - sizeof(flashpage_age);
    }
  }

  lineindexend = lineindexstart;
  return (unsigned char**)lineindexend;
}

static unsigned char flashstore_compact(unsigned char len)
{
  return 0; // Not yet
}

static void flashstore_write(unsigned char* mem, unsigned char* value, unsigned char size)
{
#ifdef SIMULATE_FLASH
  memcpy(mem, value, size);
  FILE* fp = fopen("/tmp/flashstore", "w");
  fwrite(__store, FLASHSTORE_LEN, sizeof(char), fp);
  fclose(fp);
#else
#error "Not yet"
#endif
}

static void flashstore_erase(unsigned char* mem)
{
#ifdef SIMULATE_FLASH
  memset(mem, 0xFF, FLASHSTORE_PAGESIZE);
  FILE* fp = fopen("/tmp/flashstore", "w");
  fwrite(__store, FLASHSTORE_LEN, sizeof(char), fp);
  fclose(fp);
#else
#error "Not yet"
#endif
}
