/**************************************************************************************************
  Filename:       BlueBasic.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    Basic interpreter for the CC254x BLE device.

  Copyright 2014 Tim Wilkinson. All rights reserved.

  Based on sample code with the following copyright notice:

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "linkdb.h"

#include "observer.h"
#include "peripheral.h"

#include "gapbondmgr.h"

#include "BlueBasic.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

#include "os.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms, 1600=1s)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

#define INVALID_CONNHANDLE                    0xFFFF

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 blueBasic_TaskID;   // Task ID for internal task/event processing

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void ble_connection_status(uint16 connHandle, uint8 changeType, int8 rssi);


/*********************************************************************
 * LOCAL VARIABLES
 */

#ifdef ENABLE_BLE_CONSOLE

#include "gatt_uuid.h"
#include "gap.h"

#define CONSOLE_PROFILE_SERV_UUID 0x3A, 0xA7, 0xBD, 0x64, 0x0a, 0xF7, 0xA3, 0xB5, 0x8D, 0x44, 0x16, 0x16, 0x91, 0x9E, 0xFB, 0x25
static CONST uint8 consoleProfileServUUID[] = { CONSOLE_PROFILE_SERV_UUID };
static CONST gattAttrType_t consoleProfileService = { 16, consoleProfileServUUID };
static CONST uint8 inputUUID[] = { 0xB2, 0x07, 0xCF, 0x1D, 0x47, 0x2F, 0x49, 0x37, 0xB5, 0x9F, 0x6B, 0x67, 0xE2, 0xC9, 0xFB, 0xC3 };
static CONST uint8 inputProps = GATT_PROP_READ|GATT_PROP_NOTIFY;
static CONST uint8 outputUUID[] = { 0x6D, 0x7E, 0xE5, 0x7D, 0xFB, 0x7A, 0x4B, 0xF7, 0xB2, 0x1C, 0x92, 0xFE, 0x3C, 0x9B, 0xAF, 0xD6 };
static CONST uint8 outputProps = GATT_PROP_WRITE;
static gattCharCfg_t consoleProfileCharCfg[GATT_MAX_NUM_CONN];

unsigned char ble_console_enabled;

static struct
{
  short writelen;
  char writepending;
  uint8 write[64];
} io;

static gattAttribute_t consoleProfile[] =
{
  { { ATT_BT_UUID_SIZE, primaryServiceUUID }, GATT_PERMIT_READ, 0, (uint8*)&consoleProfileService },
  { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, (uint8*)&inputProps },
  { { 16, inputUUID }, GATT_PERMIT_READ|GATT_PERMIT_WRITE, 0, io.write },
  { { ATT_BT_UUID_SIZE, clientCharCfgUUID }, GATT_PERMIT_READ|GATT_PERMIT_WRITE, 0, (uint8*)consoleProfileCharCfg },
  { { ATT_BT_UUID_SIZE, characterUUID }, GATT_PERMIT_READ, 0, (uint8*)&outputProps },
  { { 16, outputUUID }, GATT_PERMIT_READ|GATT_PERMIT_WRITE, 0, NULL },
};

static bStatus_t consoleProfile_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr, uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen);
static bStatus_t consoleProfile_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr, uint8 *pValue, uint8 len, uint16 offset);

static CONST gattServiceCBs_t consoleProfileCB =
{
  consoleProfile_ReadAttrCB,
  consoleProfile_WriteAttrCB,
  NULL
};

static CONST uint8 consoleAdvert[] =
{
  0x02, GAP_ADTYPE_FLAGS, GAP_ADTYPE_FLAGS_GENERAL|GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  0x11, GAP_ADTYPE_128BIT_MORE, CONSOLE_PROFILE_SERV_UUID,
  0x09, GAP_ADTYPE_LOCAL_NAME_COMPLETE, 'B', 'l', 'u', 'B', 'a', 's', 'i', 'c'
};

#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void blueBasic_deviceFound( gapDeviceInfoEvent_t* device );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Link callback
static void blueBasic_HandleConnStatusCB(uint16 connHandle, uint8 changeType);
static void blueBasic_RSSIUpdate(int8 rssi);

// GAP Role Callbacks
static gapRolesCBs_t blueBasic_PeripheralCBs =
{
  NULL,                           // Profile State Change Callbacks
  blueBasic_RSSIUpdate,           // When a valid RSSI is read from controller
  blueBasic_deviceFound           // Device found
};

#ifdef GAP_BOND_MGR
// GAP Bond Manager Callbacks
static gapBondCBs_t blueBasic_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};
#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      BlueBasic_Init
 *
 * @brief   Initialization function for the Blue Basic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void BlueBasic_Init( uint8 task_id )
{
  blueBasic_TaskID = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  VOID GAP_ConfigDeviceAddr(ADDRTYPE_PRIVATE_RESOLVE, NULL);

  // Setup the GAP Peripheral Role Profile
  {
#ifdef ENABLE_BLE_CONSOLE
    uint8 initial_advertising_enable = TRUE;
    uint16 gapRole_AdvertOffTime = 1;
#else
    uint8 initial_advertising_enable = FALSE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
#endif

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, initial_advertising_enable, 0, NULL );
#ifdef ENABLE_BLE_CONSOLE
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, 0, sizeof(consoleAdvert), (void*)consoleAdvert );
#endif
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, gapRole_AdvertOffTime, 0, NULL );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE,enable_update_request, 0, NULL );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, desired_min_interval, 0, NULL );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, desired_max_interval, 0, NULL );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, desired_slave_latency, 0, NULL );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, desired_conn_timeout, 0, NULL );
  }

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

#ifdef GAP_BOND_MGR
  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }
#endif

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
#ifndef ENABLE_BLE_CONSOLE
  // See: http://e2e.ti.com/support/wireless_connectivity/f/538/p/169944/668822.aspx#664740
  HCI_EXT_ClkDivOnHaltCmd(HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT);
#endif

  // Startup with standard power settings
  HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_0_DBM);
  HCI_EXT_SetRxGainCmd(HCI_EXT_RX_GAIN_STD);

  // Setup a delayed profile startup
  osal_set_event( blueBasic_TaskID, BLUEBASIC_START_DEVICE_EVT );
}

/*********************************************************************
 * @fn      BlueBasic_ProcessEvent
 *
 * @brief   Blue Basic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 BlueBasic_ProcessEvent( uint8 task_id, uint16 events )
{
  unsigned char i;

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( blueBasic_TaskID )) != NULL )
    {
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & BLUEBASIC_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &blueBasic_PeripheralCBs );

#ifdef GAP_BOND_MGR
    // Start Bond Manager
    VOID GAPBondMgr_Register( &blueBasic_BondMgrCBs );
#endif
    // Start monitoring links
    VOID linkDB_Register( blueBasic_HandleConnStatusCB );

#ifdef ENABLE_BLE_CONSOLE
    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, consoleProfileCharCfg);
    GATTServApp_RegisterService(consoleProfile, GATT_NUM_ATTRS(consoleProfile), &consoleProfileCB);
#endif

    // Start Interpreter
    interpreter_setup();

    return ( events ^ BLUEBASIC_START_DEVICE_EVT );
  }

#ifdef ENABLE_BLE_CONSOLE
  if ( events & BLUEBASIC_CONNECTION_EVENT )
  {
    if (io.writepending)
    {
      if (io.writelen > 0)
      {
        GATTServApp_ProcessCharCfg(consoleProfileCharCfg, io.write, FALSE, consoleProfile, GATT_NUM_ATTRS(consoleProfile), INVALID_TASK_ID);
      }
      else
      {
        io.writepending = 0;
      }
    }
    return ( events ^ BLUEBASIC_CONNECTION_EVENT );
  }
#endif

  if ( events & BLUEBASIC_INPUT_AVAILABLE )
  {
    interpreter_loop();
    return (events ^ BLUEBASIC_INPUT_AVAILABLE);
  }

  if ( events & BLUEBASIC_EVENT_INTERRUPTS )
  {
    for (i = 0; i < OS_MAX_INTERRUPT; i++)
    {
      if (blueBasic_interrupts[i].linenum && (events & (BLUEBASIC_EVENT_INTERRUPT << i)))
      {
        interpreter_run(blueBasic_interrupts[i].linenum, 1);
      }
    }
    return (events ^ (events & BLUEBASIC_EVENT_INTERRUPTS));
  }

  if ( events & BLUEBASIC_EVENT_TIMERS )
  {
    for (i = 0; i < OS_MAX_TIMER; i++)
    {
      if (blueBasic_timers[i].linenum && (events & (BLUEBASIC_EVENT_TIMER << i)))
      {
        interpreter_run(blueBasic_timers[i].linenum, i == DELAY_TIMER ? 0 : 1);
      }
    }
    return (events ^ (events & BLUEBASIC_EVENT_TIMERS));
  }

  // Discard unknown events
  return 0;
}

static void blueBasic_deviceFound( gapDeviceInfoEvent_t* device )
{
  interpreter_devicefound(device->addrType, device->addr, device->rssi, device->eventType, device->dataLen, device->pEvtData);
}

static void blueBasic_HandleConnStatusCB(uint16 connHandle, uint8 changeType)
{
  if (connHandle == LOOPBACK_CONNHANDLE)
  {
    return;
  }
#ifdef ENABLE_BLE_CONSOLE
  if (changeType == LINKDB_STATUS_UPDATE_REMOVED || (changeType == LINKDB_STATUS_UPDATE_STATEFLAGS && !linkDB_Up(connHandle)))
  {
    GATTServApp_InitCharCfg(connHandle, consoleProfileCharCfg);
  }
#endif
  ble_connection_status(connHandle, changeType, 0);
}

static void blueBasic_RSSIUpdate(int8 rssi)
{
  ble_connection_status(0, LINKDB_STATUS_UPDATE_RSSI, rssi);
}

#ifdef ENABLE_BLE_CONSOLE

uint8 ble_console_write(uint8 ch)
{
  static char entered;

  if (ble_console_enabled)
  {
    if (entered && io.writelen >= sizeof(io.write))
    {
      io.write[sizeof(io.write) - 1] = '*';
      return 0;
    }
    entered = 1;

    while (io.writelen >= sizeof(io.write))
    {
      osal_run_system();
    }

    io.write[io.writelen++] = ch;

    if (!io.writepending && (ch == '\n' || io.writelen == sizeof(io.write)))
    {
      io.writepending = 1;
      GATTServApp_ProcessCharCfg(consoleProfileCharCfg, io.write, FALSE, consoleProfile, GATT_NUM_ATTRS(consoleProfile), INVALID_TASK_ID);
    }

    entered = 0;
  }
  return 1;
}

static bStatus_t consoleProfile_ReadAttrCB(uint16 connHandle, gattAttribute_t *pAttr, uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen)
{
  if (io.writelen > 0)
  {
    if (maxLen > io.writelen)
    {
      maxLen = io.writelen;
    }
    OS_memcpy(pValue, io.write, maxLen);
    *pLen = maxLen;
    io.writelen -= maxLen;
    if (io.writelen > 0)
    {
      OS_memcpy(io.write, io.write + maxLen, io.writelen);
    }
  }
  else
  {
    *pLen = 0;
  }
  return SUCCESS;
}

static bStatus_t consoleProfile_WriteAttrCB(uint16 connHandle, gattAttribute_t *pAttr, uint8 *pValue, uint8 len, uint16 offset)
{
  unsigned char i;
  bStatus_t status;

  if (pAttr->type.len == ATT_BT_UUID_SIZE && BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]) == GATT_CLIENT_CHAR_CFG_UUID)
  {
    status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY);
    if (status == SUCCESS)
    {
      status = GAPRole_SendUpdateParam(32, 64, DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_CONN_TIMEOUT, GAPROLE_RESEND_PARAM_UPDATE);
      io.writelen = 0;
      io.writepending = 0;
      ble_console_enabled = 1;
      OS_timer_stop(DELAY_TIMER);
      interpreter_banner();
    }
    return status;
  }
  for (i = 0; i < len; i++)
  {
    OS_type(pValue[i]);
  }
  return SUCCESS;
}

#endif

HAL_ISR_FUNCTION(port0Isr, P0INT_VECTOR)
{
  unsigned char status;
  unsigned char i;

  HAL_ENTER_ISR();

  status = P0IFG;
  status &= P0IEN;
  if (status)
  {
    P0IFG = ~status;
    P0IF = 0;

    for (i = 0; i < OS_MAX_INTERRUPT; i++)
    {
      if (blueBasic_interrupts[i].pin < 0x008 && (status & (1 << (blueBasic_interrupts[i].pin & 7))))
      {
        osal_set_event(blueBasic_TaskID, BLUEBASIC_EVENT_INTERRUPT << i);
      }
    }
  }

  HAL_EXIT_ISR();
}

#ifdef ENABLE_BLE_CONSOLE // Not using P1 interrupts for UART

HAL_ISR_FUNCTION(port1Isr, P1INT_VECTOR)
{
  unsigned char status;
  unsigned char i;

  HAL_ENTER_ISR();

  status = P1IFG;
  status &= P1IEN;
  if (status)
  {
    P1IFG = ~status;
    P1IF = 0;

    for (i = 0; i < OS_MAX_INTERRUPT; i++)
    {
      if (blueBasic_interrupts[i].pin >= 0x100 && blueBasic_interrupts[i].pin < 0x108 && (status & (1 << (blueBasic_interrupts[i].pin & 7))))
      {
        osal_set_event(blueBasic_TaskID, BLUEBASIC_EVENT_INTERRUPT << i);
      }
    }
  }

  HAL_EXIT_ISR();
}

#endif // ENABLE_BLE_CONSOLE

HAL_ISR_FUNCTION(port2Isr, P2INT_VECTOR)
{
  unsigned char status;
  unsigned char i;

  HAL_ENTER_ISR();

  status = P2IFG;
  status &= P2IEN;
  if (status)
  {
    P2IFG = ~status;
    P2IF = 0;

    for (i = 0; i < OS_MAX_INTERRUPT; i++)
    {
      if (blueBasic_interrupts[i].pin >= 0x200 && (status & (1 << (blueBasic_interrupts[i].pin & 7))))
      {
        osal_set_event(blueBasic_TaskID, BLUEBASIC_EVENT_INTERRUPT << i);
      }
    }
  }

  HAL_EXIT_ISR();
}

/*********************************************************************
*********************************************************************/
