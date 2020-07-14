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

#define COFFEE							1
#define FLASH_CONF_FW_ADDR				(CC2538_DEV_FLASH_ADDR + 10*1024)
#define FLASH_CONF_FW_SIZE 				(0x7f800 - 10*1024)

#define COFFEE_CONF_START				CC2538_DEV_FLASH_ADDR
#define COFFEE_CONF_SIZE        		(10*1024)
#define COFFEE_CONF_MICRO_LOGS  		1
#define COFFEE_CONF_APPEND_ONLY 		0


/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 1
#endif

#define UIP_CONF_ND6_SEND_RA            1
#define UIP_CONF_ND6_SEND_NS  			1
#define UIP_CONF_ND6_SEND_NA  			1
#define UIP_CONF_IP_FORWARD             0



#define PLATFORM_HAS_UART_1				1

#define UART1_RX_PORT           		GPIO_B_NUM
#define UART1_RX_PIN            		6

#define UART1_TX_PORT           		GPIO_B_NUM
#define UART1_TX_PIN            		7

#define UART1_CONF_BAUD_RATE   			9600 

#define PLATFORM_HAS_HCHO_SENSOR		1

/** @} */
/*---------------------------------------------------------------------------*/
/** \name Switch configuration
 *
 * Switches on the Pure-X are connected as follows:
 */

#define PLATFORM_HAS_RELAY_SWITCH		1
/** INDICATE_CTRL -> PD0 */
#define LED_CTRL_PORT     GPIO_D_NUM
#define LED_CTRL_PIN      0

#define PLATFORM_RELAY_SWITCH_MAP 		\
{ \
	/**                   PORT   PIN **/ \
	/** SWITCH0 -> PD3 */ {GPIO_D_NUM, 3}, \
	/** SWITCH1 -> PD1 */ {GPIO_D_NUM, 1}, \
	/** SWITCH2 -> PB0 */ {GPIO_B_NUM, 0}, \
	/** SWITCH3 -> PB1 */ {GPIO_B_NUM, 1}, \
	/** SWITCH4 -> PB2 */ {GPIO_B_NUM, 2}, \
	/** SWITCH5 -> PB3 */ {GPIO_B_NUM, 3}, \
	/** SWITCH6 -> PB4 */ {GPIO_B_NUM, 4}, \
	/** SWITCH7 -> PB5 */ {GPIO_B_NUM, 5}  \
}



#define PLATFORM_HAS_LEDS       		1

#define LEDS_BLUE_CTRL        			0 

#define LEDS_CONF_ALL           		0

#define PLATFORM_LEDS_MAP 		\
{ \
	/**                 PORT   PIN **/ \
	/** LED0 -> PD0 */ {GPIO_D_NUM, 0}, \
}



#define PLATFORM_HAS_BUTTON     		0

/*---------------------------------------------------------------------------*/
/**
 * \name ADC configuration
 *
 * These values configure which CC2538 pins and ADC channels to use for the ADC
 * inputs.
 *
 * ADC inputs can only be on port A.
 * @{
 */
#define ADC_ALS_PWR_PORT        GPIO_A_NUM /**< ALS power GPIO control port */
#define ADC_ALS_PWR_PIN         7 /**< ALS power GPIO control pin */
#define ADC_ALS_OUT_PIN         6 /**< ALS output ADC input pin on port A */
/** @} */



#endif /* __PROJECT_CONF_H__ */
