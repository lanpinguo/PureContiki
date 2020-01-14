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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-web-demo
 * @{
 *
 * \file
 *  CoAP resource handler for CC26XX software and hardware version
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "rest-engine.h"
#include "er-coap.h"

#include "sys/clock.h"
#include "coap-server.h"

#include <string.h>


#define IPV6ADDR_FORMAT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define IPV6ADDR(addr) ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15]

/*---------------------------------------------------------------------------*/
static void
res_get_handler_nbr(void *request, void *response, uint8_t *buffer,
                   uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;

  REST.get_header_accept(request, &accept);

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
	uip_ds6_nbr_t *nbr;
	int num;
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", CONTIKI_VERSION_STRING);
  

	num = 0;
	for(nbr = nbr_table_head(ds6_neighbors);
	  nbr != NULL;
	  nbr = nbr_table_next(ds6_neighbors, nbr))
	{
		printf("\r\nNBR-%d:[",num);
		num = snprintf((char *)(buffer + num), REST_MAX_CHUNK_SIZE - num,IPV6ADDR_FORMAT, IPV6ADDR(&nbr->ipaddr));
	}

    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, coap_server_supported_msg,
                              strlen(coap_server_supported_msg));
  }
}
/*---------------------------------------------------------------------------*/
RESOURCE(res_nbr,
         "title=\"nbr list\";rt=\"text\"",
         res_get_handler_nbr,
         NULL,
         NULL,
         NULL);
/*---------------------------------------------------------------------------*/
/** @} */
