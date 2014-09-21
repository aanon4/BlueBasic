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
#include "hal_uart.h"
#include "OSAL_PwrMgr.h"
#ifdef FEATURE_OAD_HEADER
#include "oad.h"
#include "oad_target.h"
#include "hal_flash.h"
#endif
#include "hal_uart.h"

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

// Timers
#define NR_TIMERS ((OS_MAX_TIMER) - 1)
struct
{
  unsigned short lineno;
} timers[OS_MAX_TIMER];

// Serial
os_serial_t serial[OS_MAX_SERIAL];

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
  ble_console_write(ch);
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
  return -1;
}

unsigned char OS_file_read(unsigned char* buf, short len)
{
  return 0;
}

unsigned char OS_file_write(unsigned char* buf, short len)
{
  return 0;
}

void OS_file_close(void)
{
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

static void _uartCallback(uint8 port, uint8 event)
{
#ifdef HAL_UART_RX_WAKEUP
  // UART woken up. This happens in the interrupt handler so we really
  // don't want to do anything else.
  if (event == HAL_UART_RX_WAKEUP)
  {
    return;
  }
#endif
  if (port == HAL_UART_PORT_0 && (serial[0].onread || serial[0].onwrite))
  {
    osal_set_event(blueBasic_TaskID, BLUEBASIC_EVENT_SERIAL);
  }
}


unsigned char OS_serial_open(unsigned char port, unsigned long baud, unsigned char parity, unsigned char bits, unsigned char stop, unsigned char flow, unsigned short onread, unsigned short onwrite)
{
  halUARTCfg_t config;
  
  switch (baud)
  {
    case 9600:
      baud = HAL_UART_BR_9600;
      break;
    case 19200:
      baud = HAL_UART_BR_19200;
      break;
    case 38400:
      baud = HAL_UART_BR_38400;
      break;
    case 57600:
      baud = HAL_UART_BR_57600;
      break;
    case 115200:
      baud = HAL_UART_BR_115200;
      break;
    default:
      return 2;
  }
 
  // Only support port 0, no-parity, 8-bits, 1 stop bit
  if (port != 0 || parity != 'N' || bits != 8 || stop != 1 || !(flow == 'H' || flow == 'N'))
  {
    return 3;
  }

  config.configured = 1;
  config.baudRate = baud;
  config.flowControl = (flow == 'H' ? 1 : 0);
  config.flowControlThreshold = 64;
  config.idleTimeout = 0;
  config.rx.maxBufSize = 128;
  config.tx.maxBufSize = 128;
  config.intEnable = 1;
  config.callBackFunc = _uartCallback;
  if (HalUARTOpen(HAL_UART_PORT_0, &config) == HAL_UART_SUCCESS)
  {
    serial[0].onread = onread;
    serial[0].onwrite = onwrite;
    return 0;
  }
 
  return 1;
}

unsigned char OS_serial_close(unsigned char port)
{
  serial[0].onread = 0;
  serial[0].onwrite = 0;
  // HalUARTClose(0); - In the hal_uart.h include file, but not actually in the code
  return 1;
}

short OS_serial_read(unsigned char port)
{
  unsigned char ch;
  if (HalUARTRead(HAL_UART_PORT_0, &ch, 1) == 1)
  {
    return ch;
  }
  else
  {
    return -1;
  }
}

unsigned char OS_serial_write(unsigned char port, unsigned char ch)
{
  return HalUARTWrite(HAL_UART_PORT_0, &ch, 1) == 1 ? 1 : 0;
}

unsigned char OS_serial_available(unsigned char port, unsigned char ch)
{
  return ch == 'R' ? Hal_UART_RxBufLen(HAL_UART_PORT_0) : Hal_UART_TxBufLen(HAL_UART_PORT_0);
}
