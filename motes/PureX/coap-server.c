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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/* Common resources */
/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
/*  res_hello, */
/*  res_push, */
/*  res_event, */
/*  res_sub; */
/*  res_dc_status_obs; */
/*  res_dc_status, */
	res_leds,
#if PLATFORM_HAS_RELAY_SWITCH
	res_relay_switch,
#endif

#ifdef CO2
	res_dc_co2,
#endif
	res_dc_status_obs,
	res_dc_vdc,
	res_dc_hwcfg;


/*---------------------------------------------------------------------------*/
static void
start_board_resources(void)
	{

	rest_activate_resource(&res_leds, "lt");
#if PLATFORM_HAS_RELAY_SWITCH
	rest_activate_resource(&res_relay_switch,"relay-sw");
#endif
#ifdef CO2
	rest_activate_resource(&res_dc_co2, "dcdc/co2");
#endif
	rest_activate_resource(&res_dc_status_obs, "dcdc/status");
	/*  rest_activate_resource(&res_dc_status, "dcdc/status"); */
	rest_activate_resource(&res_dc_vdc, "dcdc/vdc");
	rest_activate_resource(&res_dc_hwcfg, "dcdc/hwcfg");

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
