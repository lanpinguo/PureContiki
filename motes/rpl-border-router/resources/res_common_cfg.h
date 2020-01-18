/******************************************************************************

  Copyright (C), 2001-2011, Pure Co., Ltd.

 ******************************************************************************
  File Name     : res_common_cfg.h
  Version       : Initial Draft
  Author        : lanpinguo
  Created       : 2019/12/09
  Last Modified :
  Description   :  Project config file
  Function List :
              broadcast_recv
              PROCESS_THREAD
              PROCESS_THREAD
              PROCESS_THREAD
              recv_uc
  History       :
  1.Date        : 2019/12/09
    Author      : lanpinguo
    Modification: Created file

******************************************************************************/


#ifndef __RES_COMMON_CFG_H__
#define __RES_COMMON_CFG_H__

#define XMEM_ERASE_UNIT_SIZE 1024 /*1K*/

#define MAX_COAP_PAYLOAD (64 + 1)       /* +1 for the terminating zero, which is not transmitted */

#include "net/ip/uip-debug.h"

#endif /* __RES_COMMON_CFG_H__ */
