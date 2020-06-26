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
 *  CoAP resource handler for the Sensortag-CC26xx sensors
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "coap-server.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* HTU-21 sensor resources and handler: Temperature, Humidity */
static void
res_get_handler_temp(void *request, void *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{

}
/*---------------------------------------------------------------------------*/
static void
res_get_handler_humidity(void *request, void *response, uint8_t *buffer,
                             uint16_t preferred_size, int32_t *offset)
{

}
/*---------------------------------------------------------------------------*/
RESOURCE(res_temp, "title=\"Temperature\";rt=\"C\"",
         res_get_handler_temp, NULL, NULL, NULL);

RESOURCE(res_hum, "title=\"Humidity\";rt=\"%RH\"",
         res_get_handler_humidity, NULL, NULL, NULL);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** @} */
