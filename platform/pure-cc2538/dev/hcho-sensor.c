/*
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 *
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
 * \addtogroup cc2538dk-als-sensor
 * @{
 *
 * \file
 *  Driver for the SmartRF06EB ALS
 */
#include "contiki.h"
#include "ringbuf.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/uart.h"
#include "dev/hcho-sensor.h"

#include <stdint.h>
#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#define BUFSIZE	 20
static struct ringbuf rxbuf;
static uint8_t rxbuf_data[BUFSIZE];
static struct etimer et;
static int32_t hcho_ppb;



PROCESS(hcho_data_process, "HCHO Sensor Driver");


/*---------------------------------------------------------------------------*/
int
uart_line_input_byte(unsigned char c)
{
	static uint8_t overflow = 0; /* Buffer overflow: ignore until END */



	PRINTF("(%02x)",c);


	if(!overflow) {
		/* Add character */
		if(ringbuf_put(&rxbuf, c) == 0) {
			/* Buffer overflow: ignore the rest of the line */
			overflow = 1;
		}
	} else {
		/* Buffer overflowed:
		* Only (try to) add terminator characters, otherwise skip */
		if(ringbuf_put(&rxbuf, c) != 0) {
			overflow = 0;
		}
	}

	/* Wake up consumer process */
	process_poll(&hcho_data_process);
	return 1;
}



int32_t is_valid(uint8_t *data,int32_t length)
{
	int32_t i = 0;
	uint8_t checksum = 0;

	for(i = 1; i < length -1 ; i++){
		checksum += data[i];
	}
	
	checksum = (~(checksum) + 1);
        
    if (checksum != data[length - 1]){
        return 0;
	}
    else{
        return 1;
	}

}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hcho_data_process, ev, data)
{
	static char buf[BUFSIZE];
	static int ptr;

	
	PROCESS_BEGIN();

	ptr = 0;

	while(1) 
	{
		/* Fill application buffer until newline or empty */
		int c = ringbuf_get(&rxbuf);
		if(c == -1) {
			if(ptr == 0){
				/* Buffer empty, wait for poll */
				PROCESS_YIELD();
			}
			else{
				etimer_set(&et, CLOCK_SECOND / 5);
				PROCESS_YIELD();
				if(etimer_expired(&et)) {

#if DEBUG				
					static int i;
					for(i = 0; i < ptr; i++){
						printf("%02x ",buf[i]);
					}
					printf("\r\n");
#endif
					if(is_valid((uint8_t *)buf,ptr)){
		                hcho_ppb = buf[4] * 256 + buf[5];
#if DEBUG				
		                printf("HCHO(ppb) : %ld " ,hcho_ppb);
#endif
					}
					
					ptr = 0;
				}
				else{
				}
			}
			continue;
		} 
		else {
			if(ptr < BUFSIZE-1) {
				buf[ptr++] = (uint8_t)c;
			} 
			else {
				/* Ignore character (wait for EOL) */
			}
		}
	}

	PROCESS_END();
}


/*---------------------------------------------------------------------------*/
static int
value(int type)
{
	return hcho_ppb;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
	switch(type) {
		case SENSORS_HW_INIT:

		break;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(hcho_sensor, HCHO_SENSOR, value, configure, status);


void
hcho_sensor_init(int port)
{
	ringbuf_init(&rxbuf, rxbuf_data, sizeof(rxbuf_data));
	process_start(&hcho_data_process, NULL);
	uart_init(port);
	uart_set_input(port, uart_line_input_byte);
}


/** @} */
