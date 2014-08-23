////////////////////////////////////////////////////////////////////////////////
// BlueBasic
////////////////////////////////////////////////////////////////////////////////
//
// Authors:
//      Tim Wilkinson <timjwilkinson@gmail.com>
//
// os.h
//

#if __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#define __data

#define  SIMULATE_PINS 1

#define OS_init()
#define OS_memset(A, B, C)    memset(A, B, C)
#define OS_memcpy(A, B, C)    memcpy(A, B, C)
#define OS_rmemcpy(A, B, C)   memmove(A, B, C)
#define OS_srand(A)           srandom(A)
#define OS_rand()             random()
#define OS_malloc(A)          malloc(A)
#define OS_free(A)            free(A)
#define OS_putchar(A)         putchar(A)
#define OS_breakcheck()       (0)
#define OS_reboot()
#define OS_millis()           (time(0) * 1000)
#define OS_interrupt_attach(A, B) 0
#define OS_interrupt_detach(A)    0
#define OS_delaymicroseconds(A) do { } while ((void)(A), 0)

extern void OS_prompt_buffer(unsigned char* start, unsigned char* end);
extern char OS_prompt_available(void);
extern short OS_file_open(unsigned char chan, unsigned char rw);
extern unsigned char OS_file_read(unsigned char* buf, short len);
extern unsigned char OS_file_write(unsigned char* buf, short len);
extern void OS_file_close(void);
extern void OS_autorun_set(unsigned char autorun);
extern char OS_autorun_get(void);
extern void OS_timer_stop(unsigned char id);
extern char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short lineno);

#define OS_MAX_TIMER              2
#define BLUEBASIC_EVENT_TIMER     0x0001
#define DELAY_TIMER               1
#define OS_MAX_INTERRUPT          1
#define BLUEBASIC_EVENT_INTERRUPT 0x0100
#define OS_AUTORUN_TIMEOUT        5000

// Simulate various BLE structures and values

#define GATT_PERMIT_READ        1
#define GATT_PERMIT_WRITE       2

#define GATT_PROP_BCAST         1
#define GATT_PROP_READ          2
#define GATT_PROP_WRITE_NO_RSP  4
#define GATT_PROP_WRITE         8
#define GATT_PROP_NOTIFY        16
#define GATT_PROP_INDICATE      32
#define GATT_PROP_AUTHEN        64
#define GATT_PROP_EXTENDED      128

#define GATT_CLIENT_CFG_NOTIFY  1

#define SUCCESS 0
#define FAILURE 1
#define INVALID_TASK_ID 0
#define GATT_MAX_NUM_CONN 1

#define GAP_ADTYPE_FLAGS                      0x01

#define GAP_ADTYPE_16BIT_COMPLETE             0x03
#define GAP_ADTYPE_32BIT_COMPLETE             0x05
#define GAP_ADTYPE_128BIT_COMPLETE            0x07
#define GAP_ADTYPE_LOCAL_NAME_SHORT           0x08
#define GAP_ADTYPE_LOCAL_NAME_COMPLETE        0x09

#define GAP_ADTYPE_FLAGS_LIMITED              0x01
#define GAP_ADTYPE_FLAGS_GENERAL              0x02
#define GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED  0x04

#define TGAP_GEN_DISC_SCAN                    2
#define TGAP_LIM_DISC_SCAN                    3
#define TGAP_LIM_DISC_ADV_INT_MIN             6
#define TGAP_LIM_DISC_ADV_INT_MAX             7
#define TGAP_GEN_DISC_ADV_INT_MIN             8
#define TGAP_GEN_DISC_ADV_INT_MAX             9
#define TGAP_FILTER_ADV_REPORTS               35

#define GAP_DEVICE_NAME_LEN                   21
#define GGS_DEVICE_NAME_ATT                   0

#define HCI_EXT_TX_POWER_MINUS_23_DBM         0
#define HCI_EXT_TX_POWER_MINUS_6_DBM          1
#define HCI_EXT_TX_POWER_0_DBM                2
#define HCI_EXT_TX_POWER_4_DBM                3

#define LINKDB_STATUS_UPDATE_NEW              0
#define LINKDB_STATUS_UPDATE_REMOVED          1
#define LINKDB_STATUS_UPDATE_STATEFLAGS       2

#define linkDB_Up(A)                          0
#define linkDB_State(A, B)                    0

typedef struct
{
  unsigned char len;
  const unsigned char *uuid;
} gattAttrType_t;

typedef struct
{
  gattAttrType_t type;
  unsigned char permissions;
  unsigned short handle;
  unsigned char* const pValue;
} gattAttribute_t;

typedef struct
{
  void* read;
  void* write;
  void* auth;
} gattServiceCBs_t;

typedef struct gattCharCfg
{
  unsigned short connhandle;
  unsigned char value;
} gattCharCfg_t;

extern unsigned char GATTServApp_RegisterService(gattAttribute_t* attributes, unsigned short count, const void* callbacks);
extern unsigned char GATTServApp_DeregisterService(unsigned short handle, void* attr);
extern unsigned char GATTServApp_InitCharCfg(unsigned short handle, gattCharCfg_t* charcfgtbl);
extern unsigned char GATTServApp_ProcessCharCfg(gattCharCfg_t* charcfgtbl, void* pval, unsigned char auth, gattAttribute_t* attrs, unsigned short numattrs, unsigned char taskid);
extern unsigned char GATTServApp_ProcessCCCWriteReq(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char len, unsigned short offset, unsigned short validcfg);
extern unsigned char GAPRole_SetParameter(unsigned short param, unsigned long value, unsigned char len, void* addr);
extern unsigned char GAPRole_GetParameter(unsigned short param, unsigned long* shortValue, unsigned char len, void* longValue);
extern unsigned char GGS_SetParameter(unsigned short param, unsigned char len, void* addr);
extern unsigned char HCI_EXT_SetTxPowerCmd(unsigned char power);
extern unsigned char HCI_EXT_SetRxGainCmd(unsigned char gain);
extern unsigned char GAPObserverRole_StartDiscovery(unsigned char mode, unsigned char active, unsigned char whitelist);
extern unsigned char GAPObserverRole_CancelDiscovery(void);

#else /* __APPLE__ --------------------------------------------------------------------------- */


#include "OSAL.h"
#include "hal_board.h"
#include "gatt.h"
#include "gattservapp.h"
#include "gapgattserver.h"
#include "OnBoard.h"
#include "gap.h"
#include "observer.h"
#include "peripheral.h"
#include "linkdb.h"
#include "hci.h"

#if TARGET_CC2540 || TARGET_CC2541
#define TARGET_CC254X   1
#else
#error "Unknown TARGET_XXX"
#endif
#define ENABLE_DEBUG_INTERFACE  1
#define ENABLE_BLE_CONSOLE  1

extern unsigned char blueBasic_TaskID;

#ifndef ENABLE_BLE_CONSOLE
#define OS_UART_PORT           HAL_UART_PORT_1
#define OS_UART_BAUDRATE       HAL_UART_BR_115200
#endif

// Task Events
#define BLUEBASIC_START_DEVICE_EVT 0x0001
#define BLUEBASIC_CONNECTION_EVENT 0x0002
#define BLUEBASIC_INPUT_AVAILABLE 0x0004
#define OS_MAX_TIMER              4
#define DELAY_TIMER               3
#define BLUEBASIC_EVENT_TIMER     0x0010
#define BLUEBASIC_EVENT_TIMERS    0x00F0 // Num bits == OS_MAX_TIMER
#define OS_MAX_INTERRUPT          4
#define BLUEBASIC_EVENT_INTERRUPT 0x0100
#define BLUEBASIC_EVENT_INTERRUPTS 0x0F00 // Num bits == OS_MAX_INTERRUPT

#define OS_AUTORUN_TIMEOUT        5000

#define OS_MAX_FILE               16

typedef struct
{
  unsigned short pin;
  unsigned short linenum;
} os_interrupt_t;
extern os_interrupt_t blueBasic_interrupts[OS_MAX_INTERRUPT];

typedef struct
{
  unsigned short linenum;
} os_timer_t;
extern os_timer_t blueBasic_timers[OS_MAX_TIMER];


#define OS_memset(A, B, C)     osal_memset(A, B, C)
#define OS_memcpy(A, B, C)     osal_memcpy(A, B, C)
#define OS_srand(V)            VOID V
#define OS_rand()              osal_rand()
#define OS_malloc(A)           osal_mem_alloc(A)
#define OS_free(A)             osal_mem_free(A)
#define OS_reboot()            SystemReset()

extern void OS_init(void);
extern void OS_openserial(void);
extern void OS_putchar(char ch);
extern void OS_type(char ch);
extern void OS_prompt_buffer(unsigned char* start, unsigned char* end);
extern char OS_prompt_available(void);
extern void* OS_rmemcpy(void *dst, const void *src, unsigned int len);
extern short OS_file_open(unsigned char chan, unsigned char rw);
extern unsigned char OS_file_read(unsigned char* buf, short len);
extern unsigned char OS_file_write(unsigned char* buf, short len);
extern void OS_file_close(void);
extern void OS_timer_stop(unsigned char id);
extern char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short lineno);
extern char OS_interrupt_attach(unsigned char pin, unsigned short lineno);
extern char OS_interrupt_detach(unsigned char pin);
extern void OS_autorun_set(unsigned char autorun);
extern char OS_autorun_get(void);
extern long OS_millis(void);
extern void OS_delaymicroseconds(short micros);

extern void interpreter_devicefound(unsigned char addtype, unsigned char* address, signed char rssi, unsigned char eventtype, unsigned char len, unsigned char* data);

#endif /* __APPLE__ */

#define BLE_PROFILEROLE         0x0300  //!< Reading this parameter will return GAP Role type. Read Only. Size is uint8.
#define BLE_IRK                 0x8301  //!< Identity Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the IRK will be randomly generated.
#define BLE_SRK                 0x8302  //!< Signature Resolving Key. Read/Write. Size is uint8[KEYLEN]. Default is all 0, which means that the SRK will be randomly generated.
#define BLE_SIGNCOUNTER         0x0303  //!< Sign Counter. Read/Write. Size is uint32. Default is 0.
#define BLE_BD_ADDR             0x8304  //!< Device's Address. Read Only. Size is uint8[B_ADDR_LEN]. This item is read from the controller.
#define BLE_ADVERT_ENABLED      0x0305  //!< Enable/Disable Advertising. Read/Write. Size is uint8. Default is TRUE=Enabled.
#define BLE_ADVERT_OFF_TIME     0x0306  //!< Advertising Off Time for Limited advertisements (in milliseconds). Read/Write. Size is uint16. Default is 30 seconds.
#define BLE_ADVERT_DATA         0x8307  //!< Advertisement Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN].  Default is "02:01:01", which means that it is a Limited Discoverable Advertisement.
#define BLE_SCAN_RSP_DATA       0x8308  //!< Scan Response Data. Read/Write. Max size is uint8[B_MAX_ADV_LEN]. Defaults to all 0.
#define BLE_ADV_EVENT_TYPE      0x0309  //!< Advertisement Type. Read/Write. Size is uint8.  Default is GAP_ADTYPE_ADV_IND (defined in GAP.h).
#define BLE_ADV_DIRECT_TYPE     0x030A  //!< Direct Advertisement Address Type. Read/Write. Size is uint8. Default is ADDRTYPE_PUBLIC (defined in GAP.h).
#define BLE_ADV_DIRECT_ADDR     0x830B  //!< Direct Advertisement Address. Read/Write. Size is uint8[B_ADDR_LEN]. Default is NULL.
#define BLE_ADV_CHANNEL_MAP     0x030C  //!< Which channels to advertise on. Read/Write Size is uint8. Default is GAP_ADVCHAN_ALL (defined in GAP.h)
#define BLE_ADV_FILTER_POLICY   0x030D  //!< Filter Policy. Ignored when directed advertising is used. Read/Write. Size is uint8. Default is GAP_FILTER_POLICY_ALL (defined in GAP.h).
#define BLE_CONNHANDLE          0x030E  //!< Connection Handle. Read Only. Size is uint16.
#define BLE_RSSI_READ_RATE      0x030F  //!< How often to read the RSSI during a connection. Read/Write. Size is uint16. The value is in milliseconds. Default is 0 = OFF.
#define BLE_PARAM_UPDATE_ENABLE 0x0310  //!< Slave Connection Parameter Update Enable. Read/Write. Size is uint8. If TRUE then automatic connection parameter update request is sent. Default is FALSE.
#define BLE_MIN_CONN_INTERVAL   0x0311  //!< Minimum Connection Interval to allow (n * 1.25ms).  Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 7.5 milliseconds (0x0006).
#define BLE_MAX_CONN_INTERVAL   0x0312  //!< Maximum Connection Interval to allow (n * 1.25ms).  Range: 7.5 msec to 4 seconds (0x0006 to 0x0C80). Read/Write. Size is uint16. Default is 4 seconds (0x0C80).
#define BLE_SLAVE_LATENCY       0x0313  //!< Update Parameter Slave Latency. Range: 0 - 499. Read/Write. Size is uint16. Default is 0.
#define BLE_TIMEOUT_MULTIPLIER  0x0314  //!< Update Parameter Timeout Multiplier (n * 10ms). Range: 100ms to 32 seconds (0x000a - 0x0c80). Read/Write. Size is uint16. Default is 1000.
#define BLE_CONN_BD_ADDR        0x8315  //!< Address of connected device. Read only. Size is uint8[B_MAX_ADV_LEN]. Set to all zeros when not connected.
#define BLE_CONN_INTERVAL       0x0316  //!< Current connection interval.  Read only. Size is uint16.  Range is 7.5ms to 4 seconds (0x0006 to 0x0C80).  Default is 0 (no connection).
#define BLE_CONN_LATENCY        0x0317  //!< Current slave latency.  Read only.  Size is uint16.  Range is 0 to 499. Default is 0 (no slave latency or no connection).
#define BLE_CONN_TIMEOUT        0x0318  //!< Current timeout value.  Read only.  size is uint16.  Range is 100ms to 32 seconds.  Default is 0 (no connection).
#define BLE_PARAM_UPDATE_REQ    0x0319  //!< Slave Connection Parameter Update Request. Write. Size is uint8. If TRUE then connection parameter update request is sent.
#define BLE_STATE               0x031A  //!< Reading this parameter will return GAP Peripheral Role State. Read Only. Size is uint8.

#define BLE_RXGAIN              0x0F00
#define BLE_TXPOWER             0x0F01

#define _GAPROLE(V)   (0x7FFF & (V))

typedef struct
{
  unsigned short linenum;
} os_discover_t;
extern os_discover_t blueBasic_discover;

#define LINKDB_STATUS_UPDATE_RSSI 16

extern void interpreter_setup(void);
extern void interpreter_banner(void);
extern void interpreter_loop(void);
extern unsigned char interpreter_run(unsigned short gofrom, unsigned char canreturn);
extern void interpreter_timer_event(unsigned short id);
