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
 * @{
 *
 * \file
 *     A PureX-specific CoAP server
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"

#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>




/*---------------------------------------------------------------------------*/
/* Common resources */
/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
/*extern resource_t  res_hello, */
/*extern resource_t  res_push, */
/*extern resource_t  res_event, */
/*extern resource_t  res_sub; */
/*extern resource_t  res_dc_status_obs; */
/*extern resource_t  res_dc_status, */
/*extern resource_t		res_leds; */

extern resource_t		res_device_model;
extern resource_t		res_device_sw;
extern resource_t 		res_device_hw;
extern resource_t 		res_device_uptime;
extern resource_t 		res_device_cfg_reset;


extern resource_t		res_nbr;



/*---------------------------------------------------------------------------*/
static void
start_board_resources(void)
{

	rest_activate_resource(&res_nbr,"nbr");

	rest_activate_resource(&res_device_model,"model");
	rest_activate_resource(&res_device_sw, "sw");
	rest_activate_resource(&res_device_uptime, "uptime");
	rest_activate_resource(&res_device_hw, "hw");
	rest_activate_resource(&res_device_cfg_reset, "cfg-reset");

}



/*---------------------------------------------------------------------------*/
PROCESS(coap_server_process, "PureX CoAP Server");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_server_process, ev, data)
{
	PROCESS_BEGIN();

	printf("PureX CoAP Server\r\n");

	/* Initialize the REST engine. */
	rest_init_engine();

	start_board_resources();

	/* enable show switch state */
	leds_on(1);

	/* Define application-specific events here. */
	while(1) {
		PROCESS_WAIT_EVENT();
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
