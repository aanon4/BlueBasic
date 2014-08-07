/**************************************************************************************************
  Filename:       BlueBasic.h
**************************************************************************************************/

#ifndef BLUEBASIC_H
#define BLUEBASIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void BlueBasic_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 BlueBasic_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BLUEBASIC_H */
