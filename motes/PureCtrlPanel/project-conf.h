/******************************************************************************

  Copyright (C), 2001-2011, Pure Co., Ltd.

 ******************************************************************************
  File Name     : pureX.c
  Version       : Initial Draft
  Author        : lanpinguo
  Created       : 2018/9/14
  Last Modified :
  Description   :  Project config file
  Function List :
              broadcast_recv
              PROCESS_THREAD
              PROCESS_THREAD
              PROCESS_THREAD
              recv_uc
  History       :
  1.Date        : 2018/9/14
    Author      : lanpinguo
    Modification: Created file

******************************************************************************/


#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#define XMEM_ERASE_UNIT_SIZE 			1024 /*1K*/
#define COFFEE							1
#define FLASH_CONF_FW_ADDR				(CC2538_DEV_FLASH_ADDR + 10*1024)
#define FLASH_CONF_FW_SIZE 				(0x7f800 - 10*1024)

#define COFFEE_CONF_START				CC2538_DEV_FLASH_ADDR
#define COFFEE_CONF_SIZE        		(10*1024)
#define COFFEE_CONF_MICRO_LOGS  		1
#define COFFEE_CONF_APPEND_ONLY 		0

#define PLATFORM_HAS_RELAY_SWITCH		0


/*---------------------------------------------------------------------------*/
/** \name SmartRF Button configuration
 *
 * Buttons on the SmartRF06 are connected as follows:
 * - BUTTON_CANCEL -> PA2
 * - BUTTON_SELECT -> PA3
 * - BUTTON_LEFT -> PC4
 * - BUTTON_RIGHT -> PC5
 * - BUTTON_UP -> PC6
 * - BUTTON_DOWN -> PC7
 * @{
 */

/** BUTTON_SELECT -> PA2 */
#define BUTTON_CANCEL_PORT      GPIO_A_NUM
#define BUTTON_CANCEL_PIN       2
#define BUTTON_CANCEL_VECTOR    GPIO_A_IRQn

/** BUTTON_SELECT -> PA3 */
#define BUTTON_SELECT_PORT      GPIO_A_NUM
#define BUTTON_SELECT_PIN       3
#define BUTTON_SELECT_VECTOR    GPIO_A_IRQn

/** BUTTON_LEFT -> PC4 */
#define BUTTON_LEFT_PORT        GPIO_C_NUM
#define BUTTON_LEFT_PIN         4
#define BUTTON_LEFT_VECTOR      GPIO_C_IRQn

/** BUTTON_RIGHT -> PC5 */
#define BUTTON_RIGHT_PORT       GPIO_C_NUM
#define BUTTON_RIGHT_PIN        5
#define BUTTON_RIGHT_VECTOR     GPIO_C_IRQn

/** BUTTON_UP -> PC6 */
#define BUTTON_UP_PORT          GPIO_C_NUM
#define BUTTON_UP_PIN           6
#define BUTTON_UP_VECTOR        GPIO_C_IRQn

/** BUTTON_DOWN -> PC7 */
#define BUTTON_DOWN_PORT        GPIO_C_NUM
#define BUTTON_DOWN_PIN         7
#define BUTTON_DOWN_VECTOR      GPIO_C_IRQn

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON     1
/** @} */


/* Notify that we have LEDs */
#define PLATFORM_HAS_LEDS       		1

#define LEDS_YELLOW             2 /**< LED2 (Yellow) -> PC1 */
#define LEDS_RED                1 /**< LED1 (Red)  -> PC0 */

#define LEDS_CONF_ALL           LEDS_YELLOW

#define PLATFORM_LEDS_MAP 		\
{ \
	/**                 PORT   PIN **/ \
	/** LED0 -> PC1 */ {GPIO_C_NUM, 1}, \
	/** LED1 -> PB0 */ {GPIO_B_NUM, 0}, \
	/** LED2 -> PB1 */ {GPIO_B_NUM, 1}, \
	/** LED3 -> PB2 */ {GPIO_B_NUM, 2}, \
	/** LED4 -> PB3 */ {GPIO_B_NUM, 3}, \
	/** LED5 -> PB4 */ {GPIO_B_NUM, 4}, \
	/** LED6 -> PB5 */ {GPIO_B_NUM, 5}  \
}


#endif /* __PROJECT_CONF_H__ */
