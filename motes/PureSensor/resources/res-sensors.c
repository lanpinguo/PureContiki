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
#include "res_common_cfg.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

float sensors_get_temperature(void);
float sensors_get_humidity(void);
uint16_t sensors_get_co2(void);
uint16_t sensors_get_tvoc(void);


static void res_get_handler_co2(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset);
static void res_periodic_handler_co2();

static void res_get_handler_tvoc(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset);
static void res_periodic_handler_tvoc();

static void res_get_handler_temp(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset);
static void res_periodic_handler_temp();

static void res_get_handler_hum(void *request, void *response, 
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset);
static void res_periodic_handler_hum();

/*---------------------------------------------------------------------------*/

#define MAX_AGE      60
#define INTERVAL_MIN 5
#define INTERVAL_MAX (MAX_AGE - 1)
#define CHANGE       1 

static int32_t interval_counter = INTERVAL_MIN;
static float temperature_old = 0;
static float humidity_old = 0;
static uint16_t co2_old = 0;
static uint16_t tvoc_old = 0;


/*---------------------------------------------------------------------------*/
/* HTU-21 sensor resources and handler: Temperature, Humidity */
/*---------------------------------------------------------------------------*/

PERIODIC_RESOURCE(res_co2,
		"title=\"co2\";rt=\"C\"",
		res_get_handler_co2,
		NULL,
		NULL,
		NULL,
		CLOCK_SECOND,
		res_periodic_handler_co2);

PERIODIC_RESOURCE(res_tvoc,
		"title=\"tvoc\";rt=\"C\"",
		res_get_handler_tvoc,
		NULL,
		NULL,
		NULL,
		CLOCK_SECOND,
		res_periodic_handler_tvoc);


PERIODIC_RESOURCE(res_temperature,
		"title=\"Temperature\";rt=\"C\"",
		res_get_handler_temp,
		NULL,
		NULL,
		NULL,
		CLOCK_SECOND,
		res_periodic_handler_temp);



PERIODIC_RESOURCE(res_humidity, 
		"title=\"Humidity\";rt=\"%RH\"",
        res_get_handler_hum,
        NULL,
		NULL,
		NULL,
		CLOCK_SECOND,
		res_periodic_handler_hum);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


static void
res_get_handler_co2(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset)
{
	/*
	* For minimal complexity, request query and options should be ignored for GET on observable resources.
	* Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
	* This would be a TODO in the corresponding files in contiki/apps/erbium/!
	*/

	float co2 = sensors_get_co2();
	unsigned int accept = -1;
	int a,b;


	a = (int)(co2);
	b = (co2 - a) * 100;
	
	REST.get_header_accept(request, &accept);

	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%d", a, b);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	} else if(accept == REST.type.APPLICATION_JSON) {
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'co2':%d.%d}", a, b);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	} else {
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}

	PRINTF("%s",buffer);
	/*REST.set_header_max_age(response, MAX_AGE);*/

	/* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler_co2()
{
	uint16_t co2 = sensors_get_co2();

	++interval_counter;

	if((abs(co2 - co2_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
		interval_counter >= INTERVAL_MAX) {
		interval_counter = 0;
		co2_old = co2;
		/* Notify the registered observers which will trigger the res_get_handler to create the response. */
		REST.notify_subscribers(&res_co2);
	}
}

/*---------------------------------------------------------------------------*/


static void
res_get_handler_tvoc(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset)
{
	/*
	* For minimal complexity, request query and options should be ignored for GET on observable resources.
	* Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
	* This would be a TODO in the corresponding files in contiki/apps/erbium/!
	*/

	uint16_t tvoc = sensors_get_tvoc();
	unsigned int accept = -1;
	
	REST.get_header_accept(request, &accept);

	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", tvoc);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	} else if(accept == REST.type.APPLICATION_JSON) {
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'tvoc':%d}", tvoc);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	} else {
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}

	PRINTF("%s",buffer);
	/*REST.set_header_max_age(response, MAX_AGE);*/

	/* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler_tvoc()
{
	uint16_t tvoc = sensors_get_tvoc();

	++interval_counter;

	if((abs(tvoc - tvoc_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
		interval_counter >= INTERVAL_MAX) {
		interval_counter = 0;
		tvoc_old = tvoc;
		/* Notify the registered observers which will trigger the res_get_handler to create the response. */
		REST.notify_subscribers(&res_tvoc);
	}
}

static void
res_get_handler_temp(void *request, void *response,
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset)
{
	/*
	* For minimal complexity, request query and options should be ignored for GET on observable resources.
	* Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
	* This would be a TODO in the corresponding files in contiki/apps/erbium/!
	*/

	float temperature = sensors_get_temperature();
	unsigned int accept = -1;
	int a,b;


	a = (int)(temperature);
	b = (temperature - a) * 100;
	
	REST.get_header_accept(request, &accept);

	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%d", a, b);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	} else if(accept == REST.type.APPLICATION_JSON) {
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'temperature':%d.%d}", a, b);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	} else {
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}

	PRINTF("%s",buffer);
	/*REST.set_header_max_age(response, MAX_AGE);*/

	/* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler_temp()
{
	float temperature = sensors_get_temperature();

	++interval_counter;

	if((fabs(temperature - temperature_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
		interval_counter >= INTERVAL_MAX) {
		interval_counter = 0;
		temperature_old = temperature;
		/* Notify the registered observers which will trigger the res_get_handler to create the response. */
		REST.notify_subscribers(&res_temperature);
	}
}

static void
res_get_handler_hum(void *request, void *response, 
							uint8_t *buffer, uint16_t preferred_size,
							int32_t *offset)
{
	/*
	* For minimal complexity, request query and options should be ignored for GET on observable resources.
	* Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
	* This would be a TODO in the corresponding files in contiki/apps/erbium/!
	*/

	float humidity = sensors_get_humidity();
	unsigned int accept = -1;
	int a,b;


	a = (int)(humidity);
	b = (humidity - a) * 100;
	
	REST.get_header_accept(request, &accept);

	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d.%d", a, b);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	} else if(accept == REST.type.APPLICATION_JSON) {
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'humidity':%d.%d}", a, b);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	} else {
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}

	PRINTF("%s",buffer);
	/*REST.set_header_max_age(response, MAX_AGE);*/

	/* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler_hum()
{
	float humidity = sensors_get_humidity();

	++interval_counter;

	if((fabs(humidity - humidity_old) >= CHANGE && interval_counter >= INTERVAL_MIN) || 
		interval_counter >= INTERVAL_MAX) {
		interval_counter = 0;
		humidity_old = humidity;
		/* Notify the registered observers which will trigger the res_get_handler to create the response. */
		REST.notify_subscribers(&res_humidity);
	}
}


/*---------------------------------------------------------------------------*/
/** @} */
