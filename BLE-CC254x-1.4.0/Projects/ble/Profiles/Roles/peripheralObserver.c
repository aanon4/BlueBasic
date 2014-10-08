/**************************************************************************************************
  Filename:       peripheralObserver.c
  Revised:        $Date: 2013-09-23 08:15:44 -0700 (Mon, 23 Sep 2013) $
  Revision:       $Revision: 35416 $

  Description:    GAP Peripheral Role


  Copyright 2009 - 2013 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
#include "hci_tl.h"
#include "l2cap.h"
#include "gap.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"

#include "observer.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "devinfoservice.h"
#include "gapgattserver.h"
#include "os.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
// Profile Events
#define START_ADVERTISING_EVT         0x0001  // Start Advertising
#define RSSI_READ_EVT                 0x0002  // Read RSSI
#define START_CONN_UPDATE_EVT         0x0004  // Start Connection Update Procedure
#define CONN_PARAM_TIMEOUT_EVT        0x0008  // Connection Parameters Update Timeout

#define DEFAULT_ADVERT_OFF_TIME       1       // 1ms

#define RSSI_NOT_AVAILABLE            127

#define DEFAULT_MIN_CONN_INTERVAL     16      // 20 milliseconds (units of 1.25ms)
#define DEFAULT_MAX_CONN_INTERVAL     32      // 40 milliseconds

#define MIN_CONN_INTERVAL             0x0006
#define MAX_CONN_INTERVAL             0x0C80

#define DEFAULT_TIMEOUT_MULTIPLIER    1000

#define CONN_INTERVAL_MULTIPLIER      6

#define MIN_SLAVE_LATENCY             0
#define MAX_SLAVE_LATENCY             500

#define MIN_TIMEOUT_MULTIPLIER        0x000a
#define MAX_TIMEOUT_MULTIPLIER        0x0c80

#define MAX_TIMEOUT_VALUE             0xFFFF
   
#define DEFAULT_SCAN_RESULTS          8

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 gapRole_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapRole_state;
static uint8 gapObserverRoleMaxScanRes = DEFAULT_SCAN_RESULTS;

/*********************************************************************
 * Profile Parameters - reference GAPROLE_PROFILE_PARAMETERS for
 * descriptions
 */

static uint8  gapRole_profileRole;
static uint8  gapRole_IRK[KEYLEN];
static uint8  gapRole_SRK[KEYLEN];
static uint32 gapRole_signCounter;
static uint8  gapRole_bdAddr[B_ADDR_LEN];
static uint8  gapRole_AdvEnabled = TRUE;
static uint16 gapRole_AdvertOffTime = DEFAULT_ADVERT_OFF_TIME;
static uint8  gapRole_AdvertDataLen = 3;
static uint8  gapRole_AdvertData[B_MAX_ADV_LEN] =
{
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,   // AD Type = Flags
  // Limited Discoverable & BR/EDR not supported
  (GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED),
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static uint8  gapRole_ScanRspDataLen = 0;
static uint8  gapRole_ScanRspData[B_MAX_ADV_LEN] = {0};
static uint8  gapRole_AdvEventType;
static uint8  gapRole_AdvDirectType;
static uint8  gapRole_AdvDirectAddr[B_ADDR_LEN] = {0};
static uint8  gapRole_AdvChanMap;
static uint8  gapRole_AdvFilterPolicy;

static uint16 gapRole_ConnectionHandle = INVALID_CONNHANDLE;
static uint16 gapRole_RSSIReadRate = 0;

static uint8  gapRole_ConnectedDevAddr[B_ADDR_LEN] = {0};

static uint8  gapRole_ParamUpdateEnable = TRUE;
static uint16 gapRole_MinConnInterval = DEFAULT_MIN_CONN_INTERVAL;
static uint16 gapRole_MaxConnInterval = DEFAULT_MAX_CONN_INTERVAL;
static uint16 gapRole_SlaveLatency = MIN_SLAVE_LATENCY;
static uint16 gapRole_TimeoutMultiplier = DEFAULT_TIMEOUT_MULTIPLIER;

static uint16 gapRole_ConnInterval = 0;
static uint16 gapRole_ConnSlaveLatency = 0;
static uint16 gapRole_ConnTimeout = 0;

static uint8 paramUpdateNoSuccessOption = GAPROLE_NO_ACTION;

// Application callbacks
static gapRolesCBs_t *pGapRoles_AppCGs = NULL;
static gapRolesParamUpdateCB_t *pGapRoles_ParamUpdateCB = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

/*********************************************************************
 * Profile Attributes - Table
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void gapRole_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void gapRole_ProcessGAPMsg( gapEventHdr_t *pMsg );
static void gapRole_HandleParamUpdateNoSuccess( void );
static void gapRole_startConnUpdate( uint8 handleFailure );

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @brief   Set a GAP Role parameter.
 *
 * Public function defined in peripheral.h.
 */
bStatus_t GAPRole_SetParameter( uint16 param, uint32 val, uint8 len, void *pValue )
{
  bStatus_t ret = SUCCESS;
 
  if (len == 0)
  {
    switch ( param )
    {
      case GAPROLE_SIGNCOUNTER:
        gapRole_signCounter = val;
        break;

      case GAPROLE_ADVERT_ENABLED:
        {
          uint8 oldAdvEnabled = gapRole_AdvEnabled;
          gapRole_AdvEnabled = val;

          if ( (oldAdvEnabled) && (gapRole_AdvEnabled == FALSE) )
          {
            // Turn off Advertising
            if ( ( gapRole_state == GAPROLE_ADVERTISING ) 
                || ( gapRole_state == GAPROLE_CONNECTED_ADV )
                || ( gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT ) )
            {
              VOID GAP_EndDiscoverable( gapRole_TaskID );
            }
          }
          else if ( (oldAdvEnabled == FALSE) && (gapRole_AdvEnabled) )
          {
            // Turn on Advertising
            if ( (gapRole_state == GAPROLE_STARTED)
                || (gapRole_state == GAPROLE_WAITING)
                || (gapRole_state == GAPROLE_CONNECTED)
                || (gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT) )
            {
              VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
            }
          }
        }
        break;

      case GAPROLE_ADVERT_OFF_TIME:
        gapRole_AdvertOffTime = val;
        break;
      
      case GAPROLE_ADV_EVENT_TYPE:
        if ( val <= GAP_ADTYPE_ADV_LDC_DIRECT_IND )
        {
          gapRole_AdvEventType = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_ADV_DIRECT_TYPE:
        if ( val <= ADDRTYPE_PRIVATE_RESOLVE )
        {
          gapRole_AdvDirectType = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;
   
        
      case GAPROLE_ADV_CHANNEL_MAP:
        if ( val <= 0x07 )
        {
          gapRole_AdvChanMap = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_ADV_FILTER_POLICY:
        if ( val <= GAP_FILTER_POLICY_WHITE )
        {
          gapRole_AdvFilterPolicy = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_RSSI_READ_RATE:
        {
          gapRole_RSSIReadRate = val;

          if ( gapRole_RSSIReadRate && gapRole_state == GAPROLE_CONNECTED )
          {
            // Start the RSSI Reads
            VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
          }
        }
        break;

      case GAPROLE_PARAM_UPDATE_ENABLE:
        gapRole_ParamUpdateEnable = val ? TRUE : FALSE;
        break;

      case GAPROLE_MIN_CONN_INTERVAL:
        if ( val >= MIN_CONN_INTERVAL && val <= MAX_CONN_INTERVAL )
        {
          gapRole_MinConnInterval = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_MAX_CONN_INTERVAL:
        if ( val >= MIN_CONN_INTERVAL && val <= MAX_CONN_INTERVAL )
        {
          gapRole_MaxConnInterval = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_SLAVE_LATENCY:
        if ( val < MAX_SLAVE_LATENCY )
        {
          gapRole_SlaveLatency = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_TIMEOUT_MULTIPLIER:
        if ( val >= MIN_TIMEOUT_MULTIPLIER && val <= MAX_TIMEOUT_MULTIPLIER )
        {
          gapRole_TimeoutMultiplier = val;
        }
        else
        {
          ret = bleInvalidRange;
        }
        break;

      case GAPROLE_PARAM_UPDATE_REQ:
          {
            if ( val )
            {
              // Make sure we don't send an L2CAP Connection Parameter Update Request
              // command within TGAP(conn_param_timeout) of an L2CAP Connection Parameter
              // Update Response being received.
              if ( osal_get_timeoutEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT ) == 0 )
              {             
                // Start connection update procedure
                gapRole_startConnUpdate( GAPROLE_NO_ACTION );
                
                // Connection update requested by app, cancel such pending procedure (if active)
                VOID osal_stop_timerEx( gapRole_TaskID, START_CONN_UPDATE_EVT );
              }
              else
              {
                ret = blePending;
              }
            }
            else
            {
              ret = bleInvalidRange;
            }
          }
          break;
   
      case GAPOBSERVERROLE_MAX_SCAN_RES:
        gapObserverRoleMaxScanRes = val;
        break;

      case HCI_EXT_SETTXPOWER:
        switch ((int32)val)
        {
          case -23:
            HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_MINUS_23_DBM);
            break;
          case -6:
            HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_MINUS_6_DBM);
            break;
          case 0:
            HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_0_DBM);
            break;
#ifdef TARGET_CC2540
          case 4:
            HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_4_DBM);
            break;
#endif
          default:
            ret = bleInvalidRange;
            break;
        }
        break;
 
      case HCI_EXT_SETRXGAIN:
        HCI_EXT_SetRxGainCmd(val ? TRUE : FALSE);
        break;
    
      default:
        // The param value isn't part of this profile, try the GAP.
        if ( param < TGAP_PARAMID_MAX )
        {
          ret = GAP_SetParamValue( param, (uint16)val );
        }
        else
        {
          ret = INVALIDPARAMETER;
        }
        break;
    }
  }
  else
  {
    switch ( param )
    {
    default:
      ret = INVALIDPARAMETER;
      break;

    case GAPROLE_IRK:
      if ( len == KEYLEN )
      {
        VOID osal_memcpy( gapRole_IRK, pValue, KEYLEN ) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_SRK:
      if ( len == KEYLEN )
      {
        VOID osal_memcpy( gapRole_SRK, pValue, KEYLEN ) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADVERT_DATA:
      if ( len <= B_MAX_ADV_LEN )
      {
        VOID osal_memset( gapRole_AdvertData, 0, B_MAX_ADV_LEN );
        VOID osal_memcpy( gapRole_AdvertData, pValue, len );
        gapRole_AdvertDataLen = len;
        
        // Update the advertising data
        ret = GAP_UpdateAdvertisingData( gapRole_TaskID,
                              TRUE, gapRole_AdvertDataLen, gapRole_AdvertData );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_SCAN_RSP_DATA:
      if ( len <= B_MAX_ADV_LEN )
      {
        VOID osal_memset( gapRole_ScanRspData, 0, B_MAX_ADV_LEN );
        VOID osal_memcpy( gapRole_ScanRspData, pValue, len );
        gapRole_ScanRspDataLen = len;
        
        // Update the Response Data
        ret = GAP_UpdateAdvertisingData( gapRole_TaskID,
                              FALSE, gapRole_ScanRspDataLen, gapRole_ScanRspData );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_DIRECT_ADDR:
      if ( len == B_ADDR_LEN )
      {
        VOID osal_memcpy( gapRole_AdvDirectAddr, pValue, B_ADDR_LEN ) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
    }
  }

  return ( ret );
}

/*********************************************************************
 * @brief   Get a GAP Role parameter.
 *
 * Public function defined in peripheral.h.
 */
bStatus_t GAPRole_GetParameter( uint16 param, uint32* pSimpleValue, uint8 len, void *pLongValue )
{
  bStatus_t ret = SUCCESS;

  if ( pSimpleValue  )
  {
    switch ( param )
    {
      case GAPROLE_PROFILEROLE:
        *pSimpleValue = gapRole_profileRole;
        break;

      case GAPROLE_SIGNCOUNTER:
        *pSimpleValue = gapRole_signCounter;
        break;

      case GAPROLE_ADVERT_ENABLED:
        *pSimpleValue = gapRole_AdvEnabled;
        break;

      case GAPROLE_ADVERT_OFF_TIME:
        *pSimpleValue = gapRole_AdvertOffTime;
        break;

      case GAPROLE_ADV_EVENT_TYPE:
        *pSimpleValue = gapRole_AdvEventType;
        break;

      case GAPROLE_ADV_DIRECT_TYPE:
        *pSimpleValue = gapRole_AdvDirectType;
        break;

      case GAPROLE_ADV_CHANNEL_MAP:
        *pSimpleValue = gapRole_AdvChanMap;
        break;

      case GAPROLE_ADV_FILTER_POLICY:
        *pSimpleValue = gapRole_AdvFilterPolicy;
        break;

      case GAPROLE_CONNHANDLE:
        *pSimpleValue = gapRole_ConnectionHandle;
        break;

      case GAPROLE_RSSI_READ_RATE:
        *pSimpleValue = gapRole_RSSIReadRate;
        break;

      case GAPROLE_PARAM_UPDATE_ENABLE:
        *pSimpleValue = gapRole_ParamUpdateEnable;
        break;

      case GAPROLE_MIN_CONN_INTERVAL:
        *pSimpleValue = gapRole_MinConnInterval;
        break;

      case GAPROLE_MAX_CONN_INTERVAL:
        *pSimpleValue = gapRole_MaxConnInterval;
        break;

      case GAPROLE_SLAVE_LATENCY:
        *pSimpleValue = gapRole_SlaveLatency;
        break;

      case GAPROLE_TIMEOUT_MULTIPLIER:
        *pSimpleValue = gapRole_TimeoutMultiplier;
        break;

      case GAPROLE_CONN_INTERVAL:
        *pSimpleValue = gapRole_ConnInterval;
        break;

      case GAPROLE_CONN_LATENCY:
        *pSimpleValue = gapRole_ConnSlaveLatency;
        break;

      case GAPROLE_CONN_TIMEOUT:
        *pSimpleValue = gapRole_ConnTimeout;
        break;

      case GAPROLE_STATE:
        *pSimpleValue = gapRole_state;
        break;

      case GAPOBSERVERROLE_MAX_SCAN_RES:
        *pSimpleValue = gapObserverRoleMaxScanRes;
        break;
      
      default:
        // The param value isn't part of this profile, try the GAP.
        if ( param < TGAP_PARAMID_MAX )
        {
          *pSimpleValue = GAP_GetParamValue( param );
        }
        else
        {
          ret = INVALIDPARAMETER;
        }
        break;
    }
  }
  else
  {

#define _CHECK_AND_COPY(D, L) \
    if ( len < (L) ) \
    { \
      ret = bleInvalidRange; \
    } \
    else \
    { \
      VOID osal_memset( pLongValue, 0, len ); \
      VOID osal_memcpy( pLongValue, (D), (L) ); \
    }

    switch ( param )
    {
      case GAPROLE_IRK:
        _CHECK_AND_COPY( gapRole_IRK, KEYLEN )
        break;

      case GAPROLE_BD_ADDR:
        _CHECK_AND_COPY( gapRole_bdAddr, B_ADDR_LEN )
        break;

      case GAPROLE_ADVERT_DATA:
        _CHECK_AND_COPY( gapRole_AdvertData, gapRole_AdvertDataLen )
        break;

      case GAPROLE_SCAN_RSP_DATA:
        _CHECK_AND_COPY( gapRole_ScanRspData, gapRole_ScanRspDataLen ) ;
        break;

      case GAPROLE_ADV_DIRECT_ADDR:
        _CHECK_AND_COPY( gapRole_AdvDirectAddr, B_ADDR_LEN ) ;
        break;

      case GAPROLE_SRK:
        _CHECK_AND_COPY( gapRole_SRK, KEYLEN ) ;
        break;

      case GAPROLE_CONN_BD_ADDR:
        _CHECK_AND_COPY( gapRole_ConnectedDevAddr, B_ADDR_LEN ) ;
        break;
   
      default:
        ret = INVALIDPARAMETER;
        break;
    }
  }

  return ( ret );
}

/*********************************************************************
 * @brief   Does the device initialization.
 *
 * Public function defined in peripheral.h.
 */
bStatus_t GAPRole_StartDevice( gapRolesCBs_t *pAppCallbacks )
{
  if ( gapRole_state == GAPROLE_INIT )
  {
    // Clear all of the Application callbacks
    if ( pAppCallbacks )
    {
      pGapRoles_AppCGs = pAppCallbacks;
    }

    // Start the GAP
    GAP_DeviceInit(
          gapRole_TaskID,
          gapRole_profileRole,
          gapObserverRoleMaxScanRes,
          gapRole_IRK,
          gapRole_SRK,
          &gapRole_signCounter
    );

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @brief   Register application's callbacks.
 *
 * Public function defined in peripheral.h.
 */
void GAPRole_RegisterAppCBs( gapRolesParamUpdateCB_t *pParamUpdateCB )
{
  if ( pParamUpdateCB != NULL )
  {
    pGapRoles_ParamUpdateCB = pParamUpdateCB;
  }
}

/*********************************************************************
 * @brief   Terminates the existing connection.
 *
 * Public function defined in peripheral.h.
 */
bStatus_t GAPRole_TerminateConnection( void )
{
  if ( gapRole_state == GAPROLE_CONNECTED )
  {
    return ( GAP_TerminateLinkReq( gapRole_TaskID, gapRole_ConnectionHandle, 
                                   HCI_DISCONNECT_REMOTE_USER_TERM ) );
  }
  else
  {
    return ( bleIncorrectMode );
  }
}

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * @brief   Task Initialization function.
 *
 * Internal function defined in peripheral.h.
 */
void GAPRole_Init( uint8 task_id, uint8 role )
{
  gapRole_TaskID = task_id;

  gapRole_state = GAPROLE_INIT;
  gapRole_ConnectionHandle = INVALID_CONNHANDLE;

#if 0
  GAP_RegisterForHCIMsgs( gapRole_TaskID );
#endif

  // Initialize the Profile Advertising and Connection Parameters
  gapRole_profileRole = role;

  VOID osal_memset( gapRole_IRK, 0, KEYLEN );
  VOID osal_memset( gapRole_SRK, 0, KEYLEN );
  gapRole_signCounter = 0;
  gapRole_AdvEventType = GAP_ADTYPE_ADV_IND;
  gapRole_AdvDirectType = ADDRTYPE_PUBLIC;
  gapRole_AdvChanMap = GAP_ADVCHAN_ALL;
  gapRole_AdvFilterPolicy = GAP_FILTER_POLICY_ALL;
}

/*********************************************************************
 * @brief   Task Event Processor function.
 *
 * Internal function defined in peripheral.h.
 */
uint16 GAPRole_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( gapRole_TaskID )) != NULL )
    {
      gapRole_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & GAP_EVENT_SIGN_COUNTER_CHANGED )
  {
    return ( events ^ GAP_EVENT_SIGN_COUNTER_CHANGED );
  }

  if ( events & START_ADVERTISING_EVT )
  {
    if ( gapRole_AdvEnabled )
    {
      gapAdvertisingParams_t params;

      // Setup advertisement parameters
      if ( gapRole_state == GAPROLE_CONNECTED )
      {
        // While in a connection, we can only advertise non-connectable undirected.
        params.eventType = GAP_ADTYPE_ADV_NONCONN_IND;
      }
      else
      {
        params.eventType = gapRole_AdvEventType;
        params.initiatorAddrType = gapRole_AdvDirectType;
        VOID osal_memcpy( params.initiatorAddr, gapRole_AdvDirectAddr, B_ADDR_LEN );
      }
      params.channelMap = gapRole_AdvChanMap;
      params.filterPolicy = gapRole_AdvFilterPolicy;

      if ( GAP_MakeDiscoverable( gapRole_TaskID, &params ) != SUCCESS )
      {
        gapRole_state = GAPROLE_ERROR;
        
        // Notify the application with the new state change
        if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange )
        {
          pGapRoles_AppCGs->pfnStateChange( gapRole_state );
        }
      }
    }
    return ( events ^ START_ADVERTISING_EVT );
  }

  if ( events & RSSI_READ_EVT )
  {
    // Only get RSSI when in a connection
    if ( gapRole_state == GAPROLE_CONNECTED )
    {
      // Ask for RSSI
      VOID HCI_ReadRssiCmd( gapRole_ConnectionHandle );

      // Setup next event
      if ( gapRole_RSSIReadRate )
      {
        VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
      }
    }
    return ( events ^ RSSI_READ_EVT );
  }

  if ( events & START_CONN_UPDATE_EVT )
  {
    // Start connection update procedure
    gapRole_startConnUpdate( GAPROLE_NO_ACTION );

    return ( events ^ START_CONN_UPDATE_EVT );
  }
  
  if ( events & CONN_PARAM_TIMEOUT_EVT )
  {
    // Unsuccessful in updating connection parameters
    gapRole_HandleParamUpdateNoSuccess();

    return ( events ^ CONN_PARAM_TIMEOUT_EVT );
  }
  
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      gapRole_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void gapRole_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    case HCI_GAP_EVENT_EVENT:
      if ( pMsg->status == HCI_COMMAND_COMPLETE_EVENT_CODE )
      {
        hciEvt_CmdComplete_t *pPkt = (hciEvt_CmdComplete_t *)pMsg;

        if ( pPkt->cmdOpcode == HCI_READ_RSSI )
        {
          int8 rssi = (int8)pPkt->pReturnParam[3];

          if ( (gapRole_state == GAPROLE_CONNECTED) && (rssi != RSSI_NOT_AVAILABLE) )
          {
            // Report RSSI to app
            if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnRssiRead )
            {
              pGapRoles_AppCGs->pfnRssiRead( rssi );
            }
          }
        }
      }
      break;

    case GAP_MSG_EVENT:
      gapRole_ProcessGAPMsg( (gapEventHdr_t *)pMsg );
      break;

    case L2CAP_SIGNAL_EVENT:
      {
        l2capSignalEvent_t *pPkt = (l2capSignalEvent_t *)pMsg;

        // Process the Parameter Update Response
        if ( pPkt->opcode == L2CAP_PARAM_UPDATE_RSP )
        {
          l2capParamUpdateRsp_t *pRsp = (l2capParamUpdateRsp_t *)&(pPkt->cmd.updateRsp);
                  
          if ( ( pRsp->result == L2CAP_CONN_PARAMS_REJECTED ) &&
               ( paramUpdateNoSuccessOption == GAPROLE_TERMINATE_LINK ) )
          {
            // Cancel connection param update timeout timer
            VOID osal_stop_timerEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT );
                      
            // Terminate connection immediately
            GAPRole_TerminateConnection();
          }
          else
          {
            uint16 timeout = GAP_GetParamValue( TGAP_CONN_PARAM_TIMEOUT );
            
            // Let's wait for Controller to update connection parameters if they're
            // accepted. Otherwise, decide what to do based on no success option.
            VOID osal_start_timerEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT, timeout );
          }
        }
      }
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      gapRole_ProcessGAPMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void gapRole_ProcessGAPMsg( gapEventHdr_t *pMsg )
{
  uint8 notify = FALSE;   // State changed notify the app? (default no)

  switch ( pMsg->opcode )
  {
    case GAP_DEVICE_INIT_DONE_EVENT:
      {
#ifdef FEATURE_DEVINFO
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
#endif
        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *)pMsg;
        bStatus_t stat = pPkt->hdr.status;

        if ( stat == SUCCESS )
        {
          // Save off the information
          VOID osal_memcpy( gapRole_bdAddr, pPkt->devAddr, B_ADDR_LEN );
#ifdef FEATURE_DEVINFO
          // use 6 bytes of device address for 8 bytes of system ID value
          systemId[0] = gapRole_bdAddr[0];
          systemId[1] = gapRole_bdAddr[1];
          systemId[2] = gapRole_bdAddr[2];
          // set middle bytes to zero
          systemId[4] = 0x00;
          systemId[3] = 0x00;
          // shift three bytes up
          systemId[7] = gapRole_bdAddr[5];
          systemId[6] = gapRole_bdAddr[4];
          systemId[5] = gapRole_bdAddr[3];
          DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
#endif

          gapRole_state = GAPROLE_STARTED;
          
          // This should match the original advertising setup in BlueBasic.c:consoleAdvert
          if (gapRole_AdvertDataLen == 31 && osal_memcmp(&gapRole_AdvertData[23], "BASIC#??", 8))
          {
            gapRole_AdvertData[29] = (gapRole_bdAddr[0] >> 4) + (gapRole_bdAddr[0] >= 0xA0 ? 'A' - 0xA : '0');
            gapRole_AdvertData[30] = (gapRole_bdAddr[0] & 15) + ((gapRole_bdAddr[0] & 15) >= 0x0A ? 'A' - 0xA : '0');
            GGS_SetParameter(GGS_DEVICE_NAME_ATT, 8, &gapRole_AdvertData[23]);
          }

          // Update the advertising data
          stat = GAP_UpdateAdvertisingData( gapRole_TaskID,
                              TRUE, gapRole_AdvertDataLen, gapRole_AdvertData ); 
        }

        if ( stat != SUCCESS )
        {
          gapRole_state = GAPROLE_ERROR;
        }

        notify = TRUE;
      }
      break;

    case GAP_ADV_DATA_UPDATE_DONE_EVENT:
      {
        gapAdvDataUpdateEvent_t *pPkt = (gapAdvDataUpdateEvent_t *)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
          if ( pPkt->adType )
          {
            // Setup the Response Data
            pPkt->hdr.status = GAP_UpdateAdvertisingData( gapRole_TaskID,
                              FALSE, gapRole_ScanRspDataLen, gapRole_ScanRspData );
          }
          else if ( ( gapRole_state != GAPROLE_ADVERTISING ) &&
                    ( osal_get_timeoutEx( gapRole_TaskID, START_ADVERTISING_EVT ) == 0 ) )
          {
            // Start advertising
            VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
          }
        }

        if ( pPkt->hdr.status != SUCCESS )
        {
          // Set into Error state
          gapRole_state = GAPROLE_ERROR;
          notify = TRUE;
        }
      }
      break;

    case GAP_MAKE_DISCOVERABLE_DONE_EVENT:
    case GAP_END_DISCOVERABLE_DONE_EVENT:
      {
        gapMakeDiscoverableRspEvent_t *pPkt = (gapMakeDiscoverableRspEvent_t *)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
          if ( pMsg->opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT )
          {
            if (gapRole_state == GAPROLE_CONNECTED)
            {
              gapRole_state = GAPROLE_CONNECTED_ADV;
            }
            else
            {
              gapRole_state = GAPROLE_ADVERTISING;
            }
          }
          else // GAP_END_DISCOVERABLE_DONE_EVENT
          {

            if ( gapRole_AdvertOffTime != 0 )
            {
              if ( ( gapRole_AdvEnabled ) )
              {
                VOID osal_start_timerEx( gapRole_TaskID, START_ADVERTISING_EVT, gapRole_AdvertOffTime );
              }
            }
            else
            {
              // Since gapRole_AdvertOffTime is set to 0, the device should not
              // automatically become discoverable again after a period of time.
              // Set enabler to FALSE; device will become discoverable again when
              // this value gets set to TRUE
              gapRole_AdvEnabled = FALSE;
            }
            
            if (gapRole_state == GAPROLE_CONNECTED_ADV)
            {
              // In the Advertising Off period
              gapRole_state = GAPROLE_CONNECTED;
            }
            else if (gapRole_state == GAPROLE_WAITING_AFTER_TIMEOUT)
            {
              // Advertising was just turned off after the link disconnected so begin
              // advertising again.
              gapRole_AdvEnabled = TRUE;
              
              // Turn advertising back on.
              VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT );
            }
            else
            {
              // In the Advertising Off period
              gapRole_state = GAPROLE_WAITING;
            }

          }
        }
        else
        {
          gapRole_state = GAPROLE_ERROR;
        }
        notify = TRUE;
      }
      break;

    case GAP_LINK_ESTABLISHED_EVENT:
      {
        gapEstLinkReqEvent_t *pPkt = (gapEstLinkReqEvent_t *)pMsg;

        if ( pPkt->hdr.status == SUCCESS )
        {
          VOID osal_memcpy( gapRole_ConnectedDevAddr, pPkt->devAddr, B_ADDR_LEN );
          gapRole_ConnectionHandle = pPkt->connectionHandle;
          gapRole_state = GAPROLE_CONNECTED;

          if ( gapRole_RSSIReadRate )
          {
            // Start the RSSI Reads
            VOID osal_start_timerEx( gapRole_TaskID, RSSI_READ_EVT, gapRole_RSSIReadRate );
          }

#ifdef ENABLE_BLE_CONSOLE
          HCI_EXT_ConnEventNoticeCmd(blueBasic_TaskID, BLUEBASIC_CONNECTION_EVENT);
#endif // ENABLE_BLE_CONSOLE

          // Store connection information
          gapRole_ConnInterval = pPkt->connInterval;
          gapRole_ConnSlaveLatency = pPkt->connLatency;
          gapRole_ConnTimeout = pPkt->connTimeout;

          // Check whether update parameter request is enabled
          if ( gapRole_ParamUpdateEnable == TRUE )
          {
            // Get the minimum time upon connection establishment before the 
            // peripheral can start a connection update procedure.
            uint16 timeout = GAP_GetParamValue( TGAP_CONN_PAUSE_PERIPHERAL );
            
            osal_start_timerEx( gapRole_TaskID, START_CONN_UPDATE_EVT, timeout*1000 );
          }
#ifdef GAP_BOND_MGR
          // Notify the Bond Manager to the connection
          VOID GAPBondMgr_LinkEst( pPkt->devAddrType, pPkt->devAddr, pPkt->connectionHandle, GAP_PROFILE_PERIPHERAL );
#endif         
          // Set enabler to FALSE; device will become discoverable again when
          // this value gets set to TRUE
          gapRole_AdvEnabled = FALSE;          
        }
        else if ( pPkt->hdr.status == bleGAPConnNotAcceptable )
        {
          // Set enabler to FALSE; device will become discoverable again when
          // this value gets set to TRUE
          gapRole_AdvEnabled = FALSE;

          // Go to WAITING state, and then start advertising
          gapRole_state = GAPROLE_WAITING;
        }
        else
        {
          gapRole_state = GAPROLE_ERROR;
        }
        notify = TRUE;
      }
      break;

    case GAP_LINK_TERMINATED_EVENT:
      {
        gapTerminateLinkEvent_t *pPkt = (gapTerminateLinkEvent_t *)pMsg;
#ifdef GAP_BOND_MGR
        VOID GAPBondMgr_ProcessGAPMsg( (gapEventHdr_t *)pMsg );
#endif
        osal_memset( gapRole_ConnectedDevAddr, 0, B_ADDR_LEN );

        // Erase connection information
        gapRole_ConnInterval = 0;
        gapRole_ConnSlaveLatency = 0;
        gapRole_ConnTimeout = 0;

        // Cancel all connection parameter update timers (if any active)
        VOID osal_stop_timerEx( gapRole_TaskID, START_CONN_UPDATE_EVT );
        VOID osal_stop_timerEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT );
          
        // Go to WAITING state, and then start advertising
        if( pPkt->reason == LL_SUPERVISION_TIMEOUT_TERM )
        {
          gapRole_state = GAPROLE_WAITING_AFTER_TIMEOUT;
        }
        else
        {
          gapRole_state = GAPROLE_WAITING;
        }

        notify = TRUE;
        
        // Check if still advertising from within last connection.
        if ( gapRole_AdvEnabled)
        {
          // End advertising so we can restart advertising in order 
          // to change to connectable advertising from nonconnectable.
          VOID GAP_EndDiscoverable( gapRole_TaskID );
        }
        else  // Turn advertising back on.
        {
          gapRole_AdvEnabled = TRUE;
          VOID osal_set_event( gapRole_TaskID, START_ADVERTISING_EVT);
        }

        gapRole_ConnectionHandle = INVALID_CONNHANDLE;
      }
      break;

    case GAP_LINK_PARAM_UPDATE_EVENT:
      {
        gapLinkUpdateEvent_t *pPkt = (gapLinkUpdateEvent_t *)pMsg;

        // Cancel connection param update timeout timer (if active)
        VOID osal_stop_timerEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT );
        
        if ( pPkt->hdr.status == SUCCESS )
        {
          // Store new connection parameters
          gapRole_ConnInterval = pPkt->connInterval;
          gapRole_ConnSlaveLatency = pPkt->connLatency;
          gapRole_ConnTimeout = pPkt->connTimeout;
          
          // Make sure there's no pending connection update procedure
          if ( osal_get_timeoutEx( gapRole_TaskID, START_CONN_UPDATE_EVT ) == 0 )
          {
            // Notify the application with the new connection parameters
            if ( pGapRoles_ParamUpdateCB != NULL )
            {
              (*pGapRoles_ParamUpdateCB)( gapRole_ConnInterval, 
                                          gapRole_ConnSlaveLatency, 
                                          gapRole_ConnTimeout );
            }
          }
        }
      }
      break;
 
    case GAP_DEVICE_INFO_EVENT:
      if (pGapRoles_AppCGs && pGapRoles_AppCGs->pfnDeviceFound)
      {
        pGapRoles_AppCGs->pfnDeviceFound((gapDeviceInfoEvent_t *) pMsg);
      }
      break;

    default:
      break;
  }

  if ( notify == TRUE )
  {
    // Notify the application with the new state change
    if ( pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange )
    {
      pGapRoles_AppCGs->pfnStateChange( gapRole_state );
    }
  }
}

/*********************************************************************
 * @fn      gapRole_HandleParamUpdateNoSuccess
 *
 * @brief   Handle unsuccessful connection parameters update.
 *
 * @param   none
 *
 * @return  none
 */
static void gapRole_HandleParamUpdateNoSuccess( void )
{
  // See which option was choosen for unsuccessful updates
  switch ( paramUpdateNoSuccessOption )
  {
    case GAPROLE_RESEND_PARAM_UPDATE:
      GAPRole_SendUpdateParam( gapRole_MinConnInterval, gapRole_MaxConnInterval,
                               gapRole_SlaveLatency, gapRole_TimeoutMultiplier,
                               GAPROLE_RESEND_PARAM_UPDATE );
      break;

    case GAPROLE_TERMINATE_LINK:
      GAPRole_TerminateConnection();
      break;

    case GAPROLE_NO_ACTION:
      // fall through
    default:
      //do nothing
      break;
  }
}

/********************************************************************
 * @fn          gapRole_startConnUpdate
 *
 * @brief       Start the connection update procedure
 *
 * @param       handleFailure - what to do if the update does not occur.
 *              Method may choose to terminate connection, try again, or take no action
 *
 * @return      none
 */
static void gapRole_startConnUpdate( uint8 handleFailure )
{
  // First check the current connection parameters versus the configured parameters
  if ( (gapRole_ConnInterval < gapRole_MinConnInterval)   ||
       (gapRole_ConnInterval > gapRole_MaxConnInterval)   ||
       (gapRole_ConnSlaveLatency != gapRole_SlaveLatency) ||
       (gapRole_ConnTimeout  != gapRole_TimeoutMultiplier) )
  {
    l2capParamUpdateReq_t updateReq;
    uint16 timeout = GAP_GetParamValue( TGAP_CONN_PARAM_TIMEOUT );
        
    updateReq.intervalMin = gapRole_MinConnInterval;
    updateReq.intervalMax = gapRole_MaxConnInterval;
    updateReq.slaveLatency = gapRole_SlaveLatency;
    updateReq.timeoutMultiplier = gapRole_TimeoutMultiplier;
    
    VOID L2CAP_ConnParamUpdateReq( gapRole_ConnectionHandle, &updateReq, gapRole_TaskID );
        
    paramUpdateNoSuccessOption = handleFailure;
        
    // Let's wait for L2CAP Connection Parameters Update Response
    VOID osal_start_timerEx( gapRole_TaskID, CONN_PARAM_TIMEOUT_EVT, timeout );
  }
}

/********************************************************************
 * @fn          GAPRole_SendUpdateParam
 *
 * @brief       Update the parameters of an existing connection
 *
 * @param       minConnInterval - the new min connection interval
 * @param       maxConnInterval - the new max connection interval
 * @param       latency - the new slave latency
 * @param       connTimeout - the new timeout value
 * @param       handleFailure - what to do if the update does not occur.
 *              Method may choose to terminate connection, try again, or take no action
 *
 * @return      SUCCESS, bleNotConnected, or bleInvalidRange
 */
bStatus_t GAPRole_SendUpdateParam( uint16 minConnInterval, uint16 maxConnInterval,
                                   uint16 latency, uint16 connTimeout, uint8 handleFailure )
{    
  // If there is no existing connection no update need be sent
  if ( gapRole_state != GAPROLE_CONNECTED )
  {
    return ( bleNotConnected );
  }
  
  // Check that all parameters are in range before sending request
  if ( ( minConnInterval >= MIN_CONN_INTERVAL ) &&
       ( minConnInterval < MAX_CONN_INTERVAL  ) &&
       ( maxConnInterval >= MIN_CONN_INTERVAL ) &&
       ( maxConnInterval < MAX_CONN_INTERVAL  ) &&
       ( latency         < MAX_SLAVE_LATENCY  ) &&
       ( connTimeout     >= MIN_TIMEOUT_MULTIPLIER ) &&
       ( connTimeout     < MAX_TIMEOUT_MULTIPLIER ) )
  {
    gapRole_MinConnInterval = minConnInterval;
    gapRole_MaxConnInterval = maxConnInterval;
    gapRole_SlaveLatency = latency;
    gapRole_TimeoutMultiplier = connTimeout;

    // Start connection update procedure
    gapRole_startConnUpdate( handleFailure );

    // Connection update requested by app, cancel such pending procedure (if active)
    VOID osal_stop_timerEx( gapRole_TaskID, START_CONN_UPDATE_EVT );
              
    return ( SUCCESS );
  }

  return ( bleInvalidRange );
}

/**
 * @brief   Start a device discovery scan.
 *
 * Public function defined in observer.h.
 */
bStatus_t GAPObserverRole_StartDiscovery( uint8 mode, uint8 activeScan, uint8 whiteList )
{
  gapDevDiscReq_t params;

  params.taskID = gapRole_TaskID;
  params.mode = mode;
  params.activeScan = activeScan;
  params.whiteList = whiteList;

  return GAP_DeviceDiscoveryRequest( &params );
}

/**
 * @brief   Cancel a device discovery scan.
 *
 * Public function defined in observer.h.
 */
bStatus_t GAPObserverRole_CancelDiscovery( void )
{
  return GAP_DeviceDiscoveryCancel( gapRole_TaskID );
}


/*********************************************************************
*********************************************************************/
