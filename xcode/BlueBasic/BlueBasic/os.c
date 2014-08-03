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

static char alarmfire;
static FILE* filep;
static unsigned char* bstart;
static unsigned char* bend;

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

short OS_file_open(unsigned char chan, unsigned char rw)
{
  short len = 0;
  char filename[10];

  if (filep)
  {
    return -1;
  }

  sprintf(filename, "saved%d", chan);
  filep = fopen(filename, rw == 'r' ? "r" : "w");
  if (!filep)
  {
    return -1;
  }
  if (rw == 'r')
  {
    fseek(filep, 0L, SEEK_END);
    len = ftell(filep);
    fseek(filep, 0L, SEEK_SET);
  }
  return len;
}

void OS_file_close(void)
{
  if (filep)
  {
    fclose(filep);
    filep = NULL;
  }
}

short OS_file_read(unsigned char* buf, short len)
{
  return fread(buf, len, sizeof(unsigned char), filep);
}

short OS_file_write(unsigned char* buf, short len)
{
  return fwrite(buf, len, sizeof(unsigned char), filep);
}

void OS_autorun_set(unsigned char autorun)
{
  FILE* fp = fopen("autorun", "w");
  if (fp)
  {
    fwrite(&autorun, 1, sizeof(unsigned char), fp);
    fclose(fp);
  }
}

char OS_autorun_get(void)
{
  FILE* fp = fopen("autorun", "r");
  if (!fp)
  {
    return 0;
  }
  else
  {
    char autorun = 0;
    fread(&autorun, 1, sizeof(unsigned char), fp);
    fclose(fp);
    return autorun;
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

#ifdef ENABLE_BLE

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

unsigned char GAPRole_SetParameter(unsigned short param, unsigned char len, void* addr)
{
  return SUCCESS;
}

unsigned char GAPRole_GetParameter(unsigned short param, void* addr)
{
  *(unsigned short*)addr = 0;
  return SUCCESS;
}

unsigned char GGS_SetParameter(unsigned short param, unsigned char len, void* addr)
{
  return SUCCESS;
}

unsigned char GAP_SetParamValue(unsigned short param, unsigned short value)
{
  return SUCCESS;
}

unsigned short GAP_GetParamValue(unsigned short param)
{
  return 0;
}

#endif // ENABLE_BLE

