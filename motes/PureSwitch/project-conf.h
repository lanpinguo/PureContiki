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

#define PLATFORM_HAS_RELAY_SWITCH		1

#endif /* __PROJECT_CONF_H__ */
