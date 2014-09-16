/**************************************************************************************************
  Filename:       BlueBasic_Main.c
  Revised:        $Date: 2010-07-06 15:39:18 -0700 (Tue, 06 Jul 2010) $
  Revision:       $Revision: 22902 $

  Description:    This file contains the main and callback functions for
                  the Simple BLE Peripheral sample application.

  Copyright 2010 - 2011 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED �AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/
/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

/**************************************************************************************************
 * Image header (for OAD builds)
 **************************************************************************************************/

#ifdef FEATURE_OAD_HEADER

typedef uint8 bStatus_t;

#include "oad.h"
#include "oad_target.h"

#define OAD_FLASH_PAGE_MULT  ((uint16)(HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE))

#pragma location="IMAGE_HEADER"
const __code img_hdr_t _imgHdr = {
  0xFFFF,                     // CRC-shadow must be 0xFFFF for everything else
  OAD_IMG_VER( OAD_IMAGE_VERSION ), // 15-bit Version #, left-shifted 1; OR with Image-B/Not-A bit.
  OAD_IMG_R_AREA * OAD_FLASH_PAGE_MULT,
  {'B', 'B', 'B', 'B'},       // ImgB User-Id
  { 0xFF, 0xFF, 0xFF, 0xFF }  // Reserved
};
#pragma required=_imgHdr

#pragma location="AES_HEADER"
static const __code aes_hdr_t _aesHdr = {
 { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
 { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B },  // Dummy Nonce
 { 0xFF, 0xFF, 0xFF, 0xFF }   // Spare
};
#pragma required=_aesHdr

#endif

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();

  // Initialize board I/O
  InitBoard( OB_COLD );

  /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize LL */

  /* Initialize the operating system */
  osal_init_system();

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  // Final board initialization
  InitBoard( OB_READY );
  
#if defined ( FEATURE_BOOST_CONVERTER )
  // Turn boost converter on by default
  extern unsigned char BlueBasic_powerMode;
  BlueBasic_powerMode = 1;
#if FEATURE_BOOST_CONVERTER == P2_0
  P2DIR |= 1;
#else
#error "Unknown boost converter location"
#endif
  FEATURE_BOOST_CONVERTER = 1;
#endif

#if defined ( POWER_SAVING )
  osal_pwrmgr_device( PWRMGR_BATTERY );
#endif

  /* Start OSAL */
  osal_start_system(); // No Return from here

  return 0;
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/*************************************************************************************************
**************************************************************************************************/
