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


#define MAX_COAP_PAYLOAD 64 + 1       /* +1 for the terminating zero, which is not transmitted */

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#endif /* __RES_COMMON_CFG_H__ */
