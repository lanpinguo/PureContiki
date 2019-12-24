/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *     Header file for the CC26xx web demo CoAP functionality
 */
/*---------------------------------------------------------------------------*/
#include "sys/process.h"
/*---------------------------------------------------------------------------*/
#ifndef COAP_CLIENT_H_
#define COAP_CLIENT_H_
/*---------------------------------------------------------------------------*/

#define MAX_SERVER_NUM			10

enum {
	COAP_CLIENT_OWN = 1,
	COAP_CLIENT_SW  ,
		
};



typedef struct COAP_CLIENT_ARG_s
{
	int mod_id;
	int coap_conf;
	int coap_param;
	int server_id;


}COAP_CLIENT_ARG_t;


void set_remote_server_address(uint32_t server_id, uip_ipaddr_t *ipaddr);

uip_ipaddr_t * get_remote_server_address(uint32_t server_id);

/*---------------------------------------------------------------------------*/
PROCESS_NAME(coap_client_process);
/*---------------------------------------------------------------------------*/
#endif /* COAP_CLIENT_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
