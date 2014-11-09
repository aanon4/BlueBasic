//
//  os.c
//  BlueBasic
//
//  Created by tim on 7/13/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "os.h"

// Timers
#define NR_TIMERS ((OS_MAX_TIMER) - 1)
struct
{
  unsigned short lineno;
} timers[OS_MAX_TIMER];

os_discover_t blueBasic_discover;

static char alarmfire;
static unsigned char* bstart;
static unsigned char* bend;

extern unsigned char __store[];

void OS_prompt_buffer(unsigned char* start, unsigned char* end)
{
  bstart = start;
  bend = end;
}

char OS_prompt_available(void)
{
  char quote = 0;
  unsigned char* ptr = bstart;

  for (;;)
  {
    char c = getchar();
    switch (c)
    {
      case -1:
        if (feof(stdin))
        {
          return 0;
        }
        if (alarmfire)
        {
          alarmfire = 0;
          timers[0].lineno && interpreter_run(timers[0].lineno, 1);
          timers[1].lineno && interpreter_run(timers[1].lineno, 0);
        }
        break;
      case '\n':
        OS_timer_stop(DELAY_TIMER); // Stop autorun
        OS_putchar('\n');
        *ptr = '\n';
        return 1;
      default:
        if(ptr == bend)
        {
          OS_putchar('\b');
        }
        else
        {
          // Are we in a quoted string?
          if(c == quote)
          {
            quote = 0;
          }
          else if (c == '"' || c == '\'')
          {
            quote = c;
          }
          else if (quote == 0 && c >= 'a' && c <= 'z')
          {
            c = c + 'A' - 'a';
          }
          *ptr++ = c;
          OS_putchar(c);
        }
        break;
    }
  }
}

static void alarmhandler(int sig)
{
  alarmfire = 1;
}

void OS_timer_stop(unsigned char id)
{
  alarm(0);
  alarmfire = 0;
  timers[id].lineno = 0;
}

char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short lineno)
{
  static char first = 1;
  if (first)
  {
    first = 0;
    alarmfire = 0;
    struct sigaction act = { alarmhandler, 0, 0 };
    sigaction(SIGALRM, &act, NULL);
  }
  if (id >= OS_MAX_TIMER)
  {
    return 0;
  }
  timers[id].lineno = lineno;
  ualarm((useconds_t)(timeout * 1000), repeat ? (useconds_t)(timeout * 1000) : 0);
  return 1;
}

// -- BLE placeholders

unsigned char GATTServApp_RegisterService(gattAttribute_t* attributes, unsigned short count, const void* callbacks)
{
  return SUCCESS;
}

unsigned char GATTServApp_DeregisterService(unsigned short handle, void* attr)
{
  return SUCCESS;
}

unsigned char GATTServApp_InitCharCfg(unsigned short handle, gattCharCfg_t* charcfgtbl)
{
  return SUCCESS;
}

unsigned char GATTServApp_ProcessCharCfg(gattCharCfg_t* charcfgtbl, void* pval, unsigned char auth, gattAttribute_t* attrs, unsigned short numattrs, unsigned char taskid)
{
  return SUCCESS;
}

unsigned char GATTServApp_ProcessCCCWriteReq(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char len, unsigned short offset, unsigned short validcfg)
{
  return SUCCESS;
}

unsigned char GAPRole_SetParameter(unsigned short param, unsigned long val, unsigned char len, void* addr)
{
  return SUCCESS;
}

unsigned char GAPRole_GetParameter(unsigned short param, unsigned long* shortValue, unsigned char len, void* longValue)
{
  if (shortValue)
  {
    *(unsigned short*)shortValue = 0;
  }
  else
  {
    memset(longValue, 0, len);
  }
  return SUCCESS;
}

unsigned char GAPBondMgr_SetParameter(unsigned short param, unsigned long value, unsigned char len, void* addr)
{
  return SUCCESS;
}

unsigned char GAPBondMgr_GetParameter(unsigned short param, unsigned long* shortValue, unsigned char len, void* longValue)
{
  if (shortValue)
  {
    *(unsigned short*)shortValue = 0;
  }
  else
  {
    memset(longValue, 0, len);
  }
  return SUCCESS;
}

unsigned char GGS_SetParameter(unsigned short param, unsigned char len, void* addr)
{
  return SUCCESS;
}

unsigned char HCI_EXT_SetTxPowerCmd(unsigned char power)
{
  return SUCCESS;
}

unsigned char HCI_EXT_SetRxGainCmd(unsigned char gain)
{
  return SUCCESS;
}

unsigned char GAPObserverRole_StartDiscovery(unsigned char mode, unsigned char active, unsigned char whitelist)
{
  return SUCCESS;
}

unsigned char GAPObserverRole_CancelDiscovery(void)
{
  return SUCCESS;
}

unsigned char GAPRole_TerminateConnection(void)
{
  return SUCCESS;
}

void OS_flashstore_init(void)
{
  FILE* fp = fopen("/tmp/flashstore", "r");
  if (fp)
  {
    fread(__store, FLASHSTORE_LEN, sizeof(char), fp);
    fclose(fp);
  }
  else
  {
    int lastage = 1;
    const unsigned char* ptr;
    memset(__store, 0xFF, FLASHSTORE_LEN);
    for (ptr = __store; ptr < &__store[FLASHSTORE_LEN]; ptr += FLASHSTORE_PAGESIZE)
    {
      *(int*)ptr = lastage++;
    }
    
  }
}

void OS_flashstore_write(unsigned long faddr, unsigned char* value, unsigned char sizeinwords)
{
  memcpy(&__store[faddr << 2], value, sizeinwords << 2);
  FILE* fp = fopen("/tmp/flashstore", "w");
  fwrite(__store, FLASHSTORE_LEN, sizeof(char), fp);
  fclose(fp);
}

void OS_flashstore_erase(unsigned long page)
{
  memset(&__store[page << 11], 0xFF, FLASHSTORE_PAGESIZE);
  FILE* fp = fopen("/tmp/flashstore", "w");
  fwrite(__store, FLASHSTORE_LEN, sizeof(char), fp);
  fclose(fp);
}

unsigned char OS_serial_open(unsigned char port, unsigned long baud, unsigned char parity, unsigned char bits, unsigned char stop, unsigned char flow, unsigned short onread, unsigned short onwrite)
{
  return 0;
}

unsigned char OS_serial_close(unsigned char port)
{
  return 0;
}

short OS_serial_read(unsigned char port)
{
  return 255;
}

unsigned char OS_serial_write(unsigned char port, unsigned char ch)
{
  return 0;
}

unsigned char OS_serial_available(unsigned char port, unsigned char ch)
{
  return 0;
}
