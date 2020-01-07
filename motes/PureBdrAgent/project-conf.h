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


/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                 0
#endif

#define UIP_CONF_ND6_SEND_RA            1
#define UIP_CONF_ND6_SEND_NS  			1
#define UIP_CONF_ND6_SEND_NA  			1
#define UIP_CONF_IP_FORWARD             0



#define USB_ETH_CONF_ENABLE     		1
#define NXP_SNIFFER_MODE        		1

#define DATA_IN 						0x84
#define DATA_OUT 						0x02
#define INTERRUPT_IN 					0x83
#define DATA_IN_PKT_SIZE_MAX    		(USB_EP4_SIZE)
#define INT_IN_PKT_SIZE_MAX     		(USB_EP3_SIZE)
#define DATA_OUT_PKT_SIZE_MAX   		(USB_EP2_SIZE)



/* Notify that we have LEDs */
#define PLATFORM_HAS_LEDS       		1

#define LEDS_YELLOW             2 /**< LED2 (Yellow) -> PC1 */
#define LEDS_RED                1 /**< LED1 (Red)  -> PC0 */

#define LEDS_CONF_ALL           LEDS_YELLOW

#define PLATFORM_LEDS_MAP 		\
{ \
	/**                 PORT   PIN **/ \
	/** LED0 -> PC1 */ {GPIO_C_NUM, 1}, \
}


#endif /* __PROJECT_CONF_H__ */
