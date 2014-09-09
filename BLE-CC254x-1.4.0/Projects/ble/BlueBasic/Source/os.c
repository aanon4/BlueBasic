////////////////////////////////////////////////////////////////////////////////
// BlueBasic
////////////////////////////////////////////////////////////////////////////////
//
// Authors:
//      Tim Wilkinson <timjwilkinson@gmail.com>
//
// os.c
//

#include "os.h"
#include "OSAL.h"
#include "osal_snv.h"
#include "hal_uart.h"
#include "OSAL_PwrMgr.h"
#ifdef FEATURE_OAD_HEADER
#include "oad.h"
#include "oad_target.h"
#include "hal_flash.h"
#endif

#define FILE_HANDLE_PROGRAM     0
#define FILE_HANDLE_DATA        1
#define FILE_HANDLE_MAX         (OS_MAX_FILE)

#define	FILE_HEADER             (0x80)
#define FILE_PROGRAM_FIRST      (0x81)
#define FILE_PROGRAM_END        (FILE_PROGRAM_FIRST+0x20)
#define FILE_DATA_FIRST         (FILE_PROGRAM_END+1)
#define FILE_BLOCK_SIZE         (128)

extern void osalTimeUpdate(void);

extern uint8 blueBasic_TaskID;

os_interrupt_t blueBasic_interrupts[OS_MAX_INTERRUPT];
os_timer_t blueBasic_timers[OS_MAX_TIMER];
os_discover_t blueBasic_discover;

struct program_header
{
  char autorun;
  unsigned short length[OS_MAX_FILE];
};
static struct
{
  signed char h;
  char rw;
  unsigned short l;
} file = { -1, 0, 0 };

// Timers
#define NR_TIMERS ((OS_MAX_TIMER) - 1)
struct
{
  unsigned short lineno;
} timers[OS_MAX_TIMER];

enum {
  MODE_STARTUP = 0,
  MODE_NEED_INPUT,
  MODE_GOT_INPUT,
  MODE_RUNNING

};
static struct {
  char* start;
  char* end;
  char* ptr;
  char mode;
  char quote;
#ifndef ENABLE_BLE_CONSOLE
  char wakeup;
#endif
} input;

#ifdef ENABLE_BLE_CONSOLE

extern unsigned char ble_console_write(unsigned char ch);
extern unsigned char ble_console_enabled;

#endif

#define CR	'\r'
#define NL	'\n'
#define BELL	'\b'
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x03
#define CTRLH	0x08

#ifndef ENABLE_BLE_CONSOLE

static void _uartCallback(uint8 port, uint8 event)
{
  if (port != OS_UART_PORT)
  {
    return;
  }
#ifdef HAL_UART_RX_WAKEUP
  // UART woken up. This happens in the interrupt handler so we really
  // don't want to do anything else.
  if (event == HAL_UART_RX_WAKEUP)
  {
    return;
  }
#endif

  // Waiting at the command prompt. We accept characters and insert them into the input buffer
  char c;
  while (input.mode == MODE_NEED_INPUT && HalUARTRead(OS_UART_PORT, (uint8*)&c, 1) > 0)
  {
    if (!input.wakeup)
    {
      input.wakeup = 1;
      OS_timer_stop(DELAY_TIMER);
      osal_pwrmgr_task_state(blueBasic_TaskID, PWRMGR_HOLD);
      interpreter_banner();
    }
    OS_type(c);
  }
}

void OS_openserial(void)
{
  halUARTCfg_t config;
  config.configured = 1;
  config.baudRate = OS_UART_BAUDRATE;
  config.flowControl = 1;
  config.flowControlThreshold = 64;
  config.idleTimeout = 0;
  config.rx.maxBufSize = 128;
  config.tx.maxBufSize = 128;
  config.intEnable = 1;
  config.callBackFunc = _uartCallback;

  HalUARTOpen(OS_UART_PORT, &config);
}

#endif // ENABLE_BLE_CONSOLE

void OS_type(char c)
{
  switch (c)
  {
    case 0xff:
      break;
    case NL:
    case CR:
#ifdef ENABLE_CONSOLE_ECHO
      OS_putchar('\n');
#endif // ENABLE_CONSOLE_ECHO
      *input.ptr = NL;
      input.mode = MODE_GOT_INPUT;
      // Wake the interpreter now we have something new to do
      osal_set_event(blueBasic_TaskID, BLUEBASIC_INPUT_AVAILABLE);
      break;
    case CTRLH:
      if(input.ptr == input.start)
      {
        break;
      }
      if (*--input.ptr == input.quote)
      {
        input.quote = 0;
      }
#ifdef ENABLE_CONSOLE_ECHO
      OS_putchar(CTRLH);
      OS_putchar(' ');
      OS_putchar(CTRLH);
#endif // ENABLE_CONSOLE_ECHO
      break;
    default:
      if(input.ptr != input.end)
      {
        // Are we in a quoted string?
        if(c == input.quote)
        {
          input.quote = 0;
        }
        else if (c == DQUOTE || c == SQUOTE)
        {
          input.quote = c;
        }
        else if (input.quote == 0 && c >= 'a' && c <= 'z')
        {
          c = c + 'A' - 'a';
        }
        *input.ptr++ = c;
#ifdef ENABLE_CONSOLE_ECHO
        OS_putchar(c);
#endif
      }
      break;
  }
}

void OS_prompt_buffer(unsigned char* start, unsigned char* end)
{
  input.mode = MODE_NEED_INPUT;
  input.start = (char*)start;
  input.end = (char*)end;
  input.ptr = (char*)start;
  input.quote = 0;
}

char OS_prompt_available(void)
{
  if (input.mode == MODE_GOT_INPUT)
  {
    input.mode = MODE_RUNNING;
    return 1;
  }
  else
  {
    return 0;
  }
}

void OS_putchar(char ch)
{
#ifndef ENABLE_BLE_CONSOLE
  if (input.wakeup)
  {
    HalUARTWrite(OS_UART_PORT, (uint8*)&ch, 1);
  }
#else
  ble_console_write(ch);
#endif
}

void* OS_rmemcpy(void *dst, const void GENERIC *src, unsigned int len)
{
  uint8 *pDst;
  const uint8 GENERIC *pSrc;

  pSrc = len + (uint8*)src;
  pDst = len + (uint8*)dst;

  while ( len-- )
    *--pDst = *--pSrc;

  return ( pDst );
}

void OS_init(void)
{
#ifndef ENABLE_BLE_CONSOLE
  OS_openserial();
#endif
}

short OS_file_open(unsigned char chan, unsigned char rw)
{
  struct program_header header;

  if (file.h != -1 || chan > FILE_HANDLE_MAX)
  {
    return -1;
  }
  file.h = chan;
  file.rw = rw;

  if (rw == 'r')
  {
    if (osal_snv_read(FILE_HEADER, sizeof(header), &header) != SUCCESS)
    {
      file.l = 0;
    }
    else
    {
      file.l = header.length[file.h];
    }
  }
  else
  {
    file.l = 0;
  }
  return file.l;
}

unsigned char OS_file_read(unsigned char* buf, short len)
{
  if (len <= file.l)
  {
    if (file.h == FILE_HANDLE_PROGRAM)
    {
      unsigned char idx = FILE_PROGRAM_FIRST;
      while (len > 0 && idx < FILE_PROGRAM_END)
      {
        if (osal_snv_read(idx, (len > FILE_BLOCK_SIZE ? FILE_BLOCK_SIZE : len), buf) != SUCCESS)
        {
          return -1;
        }
        buf += FILE_BLOCK_SIZE;
        idx++;
        len -= FILE_BLOCK_SIZE;
      }
      return 1;
    }
    else if (osal_snv_read(file.h - FILE_HANDLE_DATA + FILE_DATA_FIRST, len, buf) == SUCCESS)
    {
      return 1;
    }
  }
  return 0;
}

unsigned char OS_file_write(unsigned char* buf, short len)
{
  if (file.h == FILE_HANDLE_PROGRAM)
  {
    unsigned char idx = FILE_PROGRAM_FIRST;
    file.l = len;
    while (len > 0 && idx < FILE_PROGRAM_END)
    {
      if (osal_snv_write(idx, (len > FILE_BLOCK_SIZE ? FILE_BLOCK_SIZE : len), buf) != SUCCESS)
      {
        file.l = 0;
        return -1;
      }
      buf += FILE_BLOCK_SIZE;
      idx++;
      len -= FILE_BLOCK_SIZE;
    }
    return 1;
  }
  else if (len <= FILE_BLOCK_SIZE)
  {
    if (osal_snv_write(file.h - FILE_HANDLE_DATA + FILE_DATA_FIRST, len, buf) == SUCCESS)
    {
      file.l = len;
      return 1;
    }
  }
  return 0;
}

void OS_file_close(void)
{
  struct program_header header;

  if (file.h != -1 && file.rw == 'w')
  {
    if (osal_snv_read(FILE_HEADER, sizeof(header), &header) != SUCCESS)
    {
      OS_memset(&header, 0, sizeof(header));
    }
    header.length[file.h] = file.l;
    osal_snv_write(FILE_HEADER, sizeof(header), &header);
  }
  file.h = -1;
}

void OS_autorun_set(unsigned char autorun)
{
  struct program_header header;

  if (osal_snv_read(FILE_HEADER, sizeof(header), &header) != SUCCESS)
  {
    OS_memset(&header, 0, sizeof(header));
  }
  header.autorun = autorun;
  osal_snv_write(FILE_HEADER, sizeof(header), &header);
}

char OS_autorun_get(void)
{
  struct program_header header;

  if (osal_snv_read(FILE_HEADER, sizeof(header), &header) != SUCCESS)
  {
    return 0;
  }
  else
  {
    return header.autorun;
  }
}

void OS_timer_stop(unsigned char id)
{
  blueBasic_timers[id].linenum = 0;
  osal_stop_timerEx(blueBasic_TaskID, BLUEBASIC_EVENT_TIMER << id);
}

char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short linenum)
{
  if (id >= OS_MAX_TIMER)
  {
    return 0;
  }
  blueBasic_timers[id].linenum = linenum;
  if (repeat)
  {
    osal_start_reload_timer(blueBasic_TaskID, BLUEBASIC_EVENT_TIMER << id, timeout);
  }
  else
  {
    osal_start_timerEx(blueBasic_TaskID, BLUEBASIC_EVENT_TIMER << id, timeout);
  }
  return 1;
}

char OS_interrupt_attach(unsigned char pin, unsigned short lineno)
{
  unsigned char i;

  for (i = 0; i < OS_MAX_INTERRUPT; i++)
  {
    if (blueBasic_interrupts[i].linenum == 0)
    {
      blueBasic_interrupts[i].pin = pin;
      blueBasic_interrupts[i].linenum = lineno;
      return 1;
    }
  }
  return 0;
}

char OS_interrupt_detach(unsigned char pin)
{
  unsigned char i;

  for (i = 0; i < OS_MAX_INTERRUPT; i++)
  {
    if (blueBasic_interrupts[i].pin == pin)
    {
      blueBasic_interrupts[i].pin = 0;
      blueBasic_interrupts[i].linenum = 0;
      return 1;
    }
  }
  return 0;
}

long OS_millis(void)
{
  osalTimeUpdate();
  return osal_GetSystemClock();
}

void OS_reboot(char flash)
{
 #ifdef FEATURE_OAD_HEADER
  if (flash)
  {
    short zero = 0;
    uint16 addr = OAD_IMG_B_PAGE * (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE);
    HalFlashWrite(addr, (uint8*)&zero, sizeof(zero));
  }
#else
  VOID flash;
#endif
  SystemReset();
}

#pragma optimize=none
void OS_delaymicroseconds(short micros)
{
  // An empty loop is pretty accurate it turns out
  while (micros-- > 0)
    ;
}

void OS_flashstore_init(void)
{
  // If flashstore is uninitialized, deleting all the pages will set it up correctly.
  if (*(unsigned long*)FLASHSTORE_CPU_BASEADDR == 0xFFFFFFFF)
  {
    flashstore_deleteall();
  }
}
