////////////////////////////////////////////////////////////////////////////////
// BlueBasic
////////////////////////////////////////////////////////////////////////////////
//
// Authors:
//      Tim Wilkinson <timjwilkinson@gmail.com>
//
// os.h
//

#define kVersion "v0.6"

#if __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <assert.h>

#define __data

#define SIMULATE_PINS   1
#define ENABLE_PORT0    1
#define ENABLE_PORT1    1
#define SIMULATE_FLASH  1

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
#define OS_reboot(F)
#define OS_get_millis()       (time(0) * 1000)
#define OS_set_millis(V)      do { } while ((void)(V), 0)
#define OS_interrupt_attach(A, B) 0
#define OS_interrupt_detach(A)    0
#define OS_delaymicroseconds(A) do { } while ((void)(A), 0)

extern void OS_prompt_buffer(unsigned char* start, unsigned char* end);
extern char OS_prompt_available(void);
extern void OS_timer_stop(unsigned char id);
extern char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short lineno);
extern void OS_flashstore_init(void);
extern void OS_flashstore_write(unsigned long faddr, unsigned char* value, unsigned char sizeinwords);
extern void OS_flashstore_erase(unsigned long page);


#define OS_MAX_TIMER              2
#define BLUEBASIC_EVENT_TIMER     0x0001
#define DELAY_TIMER               1
#define OS_MAX_INTERRUPT          1
#define BLUEBASIC_EVENT_INTERRUPT 0x0100
#define OS_AUTORUN_TIMEOUT        5000

// Simulate various BLE structures and values

#define GATT_PERMIT_READ        1
#define GATT_PERMIT_WRITE       2
#define GATT_PERMIT_AUTHEN_READ 4
#define GATT_PERMIT_AUTHEN_WRITE 8

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

#define TGAP_GEN_DISC_ADV_MIN                 0
#define TGAP_LIM_ADV_TIMEOUT                  1
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
extern unsigned char GAPRole_TerminateConnection(void);
extern unsigned char GGS_SetParameter(unsigned short param, unsigned char len, void* addr);
extern unsigned char GAPBondMgr_SetParameter(unsigned short param, unsigned long value, unsigned char len, void* addr);
extern unsigned char GAPBondMgr_GetParameter(unsigned short param, unsigned long* shortValue, unsigned char len, void* longValue);
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
#ifdef GAP_BOND_MGR
#include "gapbondmgr.h"
#endif
#include "OnBoard.h"
#include "gap.h"
#include "observer.h"
#include "peripheral.h"
#include "linkdb.h"
#include "hci.h"
#include "hal_flash.h"
#include "timestamp.h"

// Configurations
#ifdef TARGET_PETRA

#define TARGET_CC2540           1
#define ENABLE_DEBUG_INTERFACE  1
#define ENABLE_LOWPOWER_CLOCK   1
#define ENABLE_BLE_CONSOLE      1
#define ENABLE_FAKE_OAD_PROFILE 1
#define ENABLE_PORT0            1
#define ENABLE_PORT1            1
#define FEATURE_BOOST_CONVERTER P2_0

#else // TARGET_PETRA

#define ENABLE_BLE_CONSOLE      1
#define ENABLE_FAKE_OAD_PROFILE 1
#define ENABLE_PORT0            1
#define ENABLE_PORT1            1
#define ENABLE_PORT2            1

#endif // TARGET_PETRA

#if TARGET_CC2540 || TARGET_CC2541
#define TARGET_CC254X   1
#else
#error "Unknown TARGET_XXX"
#endif

extern unsigned char blueBasic_TaskID;

// Task Events
#define BLUEBASIC_START_DEVICE_EVT 0x0001
#define BLUEBASIC_CONNECTION_EVENT 0x0002
#define BLUEBASIC_INPUT_AVAILABLE 0x0004
#define BLUEBASIC_EVENT_SERIAL    0x0008
#define OS_MAX_TIMER              4
#define DELAY_TIMER               3
#define BLUEBASIC_EVENT_TIMER     0x0010
#define BLUEBASIC_EVENT_TIMERS    0x00F0 // Num bits == OS_MAX_TIMER
#define OS_MAX_INTERRUPT          4
#define BLUEBASIC_EVENT_INTERRUPT 0x0100
#define BLUEBASIC_EVENT_INTERRUPTS 0x0F00 // Num bits == OS_MAX_INTERRUPT

#define OS_AUTORUN_TIMEOUT        5000

#define OS_MAX_FILE               16

#define OS_MAX_SERIAL             1

// Serial
typedef struct
{
  unsigned short onread;
  unsigned short onwrite;
} os_serial_t;
extern os_serial_t serial[OS_MAX_SERIAL];

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

#define FLASHSTORE_CPU_BASEADDR ((unsigned char*)0x9000)
#define FLASHSTORE_DMA_BASEADDR ((unsigned long)0x29000)

#define OS_memset(A, B, C)     osal_memset(A, B, C)
#define OS_memcpy(A, B, C)     osal_memcpy(A, B, C)
#define OS_srand(V)            VOID V
#define OS_rand()              osal_rand()
#define OS_malloc(A)           osal_mem_alloc(A)
#define OS_free(A)             osal_mem_free(A)

#define OS_flashstore_write(A, V, L)  HalFlashWrite(A, V, L)
#define OS_flashstore_erase(P)        HalFlashErase(P)

extern void OS_init(void);
extern void OS_openserial(void);
extern void OS_putchar(char ch);
extern void OS_type(char ch);
extern void OS_prompt_buffer(unsigned char* start, unsigned char* end);
extern char OS_prompt_available(void);
extern void* OS_rmemcpy(void *dst, const void *src, unsigned int len);
extern void OS_timer_stop(unsigned char id);
extern char OS_timer_start(unsigned char id, unsigned long timeout, unsigned char repeat, unsigned short lineno);
extern char OS_interrupt_attach(unsigned char pin, unsigned short lineno);
extern char OS_interrupt_detach(unsigned char pin);
extern long OS_get_millis(void);
extern void OS_set_millis(long time);
extern void OS_delaymicroseconds(short micros);
extern void OS_reboot(char flash);
extern void OS_flashstore_init(void);

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

//
// GAPBOND
//
#define BLE_PAIRING_MODE        0x0400  //!< Pairing Mode: @ref  BLE_PAIRING_MODE_DEFINES. Read/Write. Size is uint8. Default is BLE_PAIRING_MODE_WAIT_FOR_REQ.
#define BLE_INITIATE_WAIT       0x0401  //!< Pairing Mode Initiate wait timeout.  This is the time it will wait for a Pairing Request before sending the Slave Initiate Request. Read/Write. Size is uint16. Default is 1000(in milliseconds).
#define BLE_MITM_PROTECTION     0x0402  //!< Man-In-The-Middle (MITM) basically turns on Passkey protection in the pairing algorithm. Read/Write. Size is uint8. Default is 0(disabled).
#define BLE_IO_CAPABILITIES     0x0403  //!< I/O capabilities.  Read/Write. Size is uint8. Default is BLE_IO_CAP_DISPLAY_ONLY @ref BLE_IO_CAP_DEFINES.
#define BLE_OOB_ENABLED         0x0404  //!< OOB data available for pairing algorithm. Read/Write. Size is uint8. Default is 0(disabled).
#define BLE_OOB_DATA            0x8405  //!< OOB Data. Read/Write. size uint8[16]. Default is all 0's.
#define BLE_BONDING_ENABLED     0x0406  //!< Request Bonding during the pairing process if enabled.  Read/Write. Size is uint8. Default is 0(disabled).
#define BLE_KEY_DIST_LIST       0x0407  //!< The key distribution list for bonding.  size is uint8.  @ref BLE_KEY_DIST_DEFINES. Default is sEncKey, sIdKey, mIdKey, mSign enabled.
#define BLE_DEFAULT_PASSCODE    0x0408  //!< The default passcode for MITM protection. size is uint32. Range is 0 - 999,999. Default is 0.
#define BLE_ERASE_ALLBONDS      0x0409  //!< Erase all of the bonded devices. Write Only. No Size.
#define BLE_KEYSIZE             0x040C  //!< Key Size used in pairing. Read/Write. size is uint8. Default is 16.
#define BLE_AUTO_SYNC_WL        0x040D  //!< Clears the White List adds to it each unique address stored by bonds in NV. Read/Write. Size is uint8. Default is FALSE.
#define BLE_BOND_COUNT          0x040E  //!< Gets the total number of bonds stored in NV. Read Only. Size is uint8. Default is 0 (no bonds).
#define BLE_BOND_FAIL_ACTION    0x040F  //!< Possible actions Central may take upon an unsuccessful bonding. Write Only. Size is uint8. Default is 0x02 (Terminate link upon unsuccessful bonding).
#define BLE_ERASE_SINGLEBOND    0x8410  //!< Erase a single bonded device. Write only. Must provide address type followed by device address.

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

#define PIN_MAKE(A,I) (((A) << 6) | ((I) << 3))
#define PIN_MAJOR(P)  ((P) >> 6)
#define PIN_MINOR(P)  (((P) >> 3) & 7)

#define FLASHSTORE_NRPAGES    4
#define FLASHSTORE_PAGESIZE   2048
#define FLASHSTORE_LEN        (FLASHSTORE_NRPAGES * FLASHSTORE_PAGESIZE)

enum
{
  FLASHID_INVALID = 0x0000,
  FLASHID_SPECIAL = 0xFFFE,
  FLASHID_FREE    = 0xFFFF,
};

enum
{
  FLASHSPECIAL_AUTORUN = 0x00000001,
  FLASHSPECIAL_SNV     = 0x00000100,
  FLASHSPECIAL_FILE0   = 0x00100000,
  FLASHSPECIAL_FILE25  = 0x00290000,
};

#define FS_NR_FILE_HANDLES 4
#define FS_MAKE_FILE_SPECIAL(NAME,OFF)  (FLASHSPECIAL_FILE0+(((unsigned long)((NAME)-'A'))<<16)|(OFF))
#define FLASHSPECIAL_NR_FILE_RECORDS 0xFFFF
#define FLASHSPECIAL_DATA_LEN       2
#define FLASHSPECIAL_ITEM_ID        3
#define FLASHSPECIAL_DATA_OFFSET    (FLASHSPECIAL_ITEM_ID + sizeof(unsigned long))

extern unsigned char** flashstore_init(unsigned char** startmem);
extern unsigned char** flashstore_addline(unsigned char* line);
extern unsigned char** flashstore_deleteline(unsigned short id);
extern unsigned char** flashstore_deleteall(void);
extern unsigned short** flashstore_findclosest(unsigned short id);
extern unsigned int flashstore_freemem(void);
extern void flashstore_compact(unsigned char asklen, unsigned char* tempmemstart, unsigned char* tempmemend);
extern unsigned char flashstore_addspecial(unsigned char* item);
extern unsigned char flashstore_deletespecial(unsigned long specialid);
extern unsigned char* flashstore_findspecial(unsigned long specialid);

extern unsigned char OS_serial_open(unsigned char port, unsigned long baud, unsigned char parity, unsigned char bits, unsigned char stop, unsigned char flow, unsigned short onread, unsigned short onwrite);
extern unsigned char OS_serial_close(unsigned char port);
extern short OS_serial_read(unsigned char port);
extern unsigned char OS_serial_write(unsigned char port, unsigned char ch);
extern unsigned char OS_serial_available(unsigned char port, unsigned char ch);
