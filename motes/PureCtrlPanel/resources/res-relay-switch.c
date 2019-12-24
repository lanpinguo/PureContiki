/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *      CoAP resource handler for the CC26xx LEDs. Slightly modified copy of
 *      the one found in Contiki's original CoAP example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch> (original)
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "dev/leds.h"
#include "relay_switch.h"
#include <string.h>
#include "res_common_cfg.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{


  /* a request comes from a remote host */
  if(request != NULL) {


    coap_packet_t *const coap_req = (coap_packet_t *)request;

    PRINTF("relay-sw  GET (%s %u)\r\n", coap_req->type == COAP_TYPE_CON ? "CON" : "NON", coap_req->mid);

	/* Code 2.05 CONTENT is default. */
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	/* REST.set_header_max_age(response, 30); */
	REST.set_response_payload(
		response,
		buffer,
		snprintf((char *)buffer, MAX_COAP_PAYLOAD, "relay-sw state: 0x%04x", 0x55AA));

  } 
}



static void
res_post_put_handler(void *request, void *response, uint8_t *buffer,
                     uint16_t preferred_size, int32_t *offset)
{
	coap_packet_t *const coap_req = (coap_packet_t *)request;
	size_t len = 0;
	const char *index = NULL;
	const char *mode = NULL;
	uint8_t sw_index = 0;
	uint8_t sw_state = 0;
	int success = 1;

	PRINTF("relay-sw PUT (%s %u) : %s\r\n", coap_req->type == COAP_TYPE_CON ? "CON" : "NON", coap_req->mid,coap_req->payload);
	if((len = REST.get_post_variable(request, "index", &index))) {
		sw_index = atoi(index);
	} else {
		success = 0;
	}

	if(success && (len = REST.get_post_variable(request, "mode", &mode))) {
		if(strncmp(mode, "on", len) == 0) {
			sw_state = 1;
		} 
		else if(strncmp(mode, "off", len) == 0) {
			sw_state = 0;
		} else {
			success = 0;
		}
	}

	printf("Put Relay-sw[%d] : %s\r\n",sw_index,mode);
	relay_switch_set(sw_index,sw_state);
	if(!success) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
	}
}
/*---------------------------------------------------------------------------*/
/*
 * A simple actuator example, depending on the color query parameter and post
 * variable mode, corresponding led is activated or deactivated
 */

RESOURCE(res_relay_switch,
         "title=\"Relay-Switch: ?index=1-8, POST/PUT mode=on|off\";rt=\"Control\"",
         res_get_handler,
         res_post_put_handler,
         res_post_put_handler,
         NULL);
/*---------------------------------------------------------------------------*/
/** @} */
