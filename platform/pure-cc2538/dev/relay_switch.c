/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 * \addtogroup 
 * @{
 *
 * \defgroup cc2538dk-leds SmartRF06EB LED driver
 *
 * Relay switch driver implementation for the Pure-X
 * @{
 *
 * \file
 * Relay switch driver implementation for the Pure-X
 */
#include "contiki.h"
#include "reg.h"
#include "dev/gpio.h"
#include "dev/ioc.h"
#include "dev/relay_switch.h"
#include "stdio.h"



#define SWITCH_NUM 	(sizeof(relay_sw_table)/sizeof(RELAY_SWITCH_t))

RELAY_SWITCH_t relay_sw_table[] = PLATFORM_RELAY_SWITCH_MAP;


/*---------------------------------------------------------------------------*/
void
relay_switch_init(void)
{
	int i;

	

	for(i = 0 ; i < SWITCH_NUM; i++){
		/* Software controlled */
		GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(relay_sw_table[i].port), (1<<relay_sw_table[i].pin));
		
		GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(relay_sw_table[i].port), (1<<relay_sw_table[i].pin));
	
	    ioc_set_over(relay_sw_table[i].port, relay_sw_table[i].pin, IOC_OVERRIDE_OE);
	}


	printf("relay_switch_init done\r\n");

}
/*---------------------------------------------------------------------------*/
uint8_t
relay_switch_get(uint8_t sw)
{
	uint32_t val;

	
	if(sw > (SWITCH_NUM - 1)){
		return 0;
	}


	val = GPIO_READ_PIN(GPIO_PORT_TO_BASE(relay_sw_table[sw].port),(1<<relay_sw_table[sw].pin));

	return (val > 0) ? 1 : 0;	
}

int32_t
relay_switch_get_all(uint32_t *state, uint32_t *mask)
{
	uint32_t val;
	int i;

	
	if(state == NULL || mask == NULL){
		return -1;
	}

	/*clean first*/
	*state = 0;
	*mask = 0;

	
	for(i = 0 ; i < SWITCH_NUM; i++){
		val = GPIO_READ_PIN(
				GPIO_PORT_TO_BASE(relay_sw_table[i].port),
				(1<<relay_sw_table[i].pin));
		if(val > 0){
			*state |= (1<<i);
		}else{
			*state <<= 1;
		}
		*mask |= (1<<i);
	}


	return 0;	
}


/*---------------------------------------------------------------------------*/
void
relay_switch_set(uint8_t sw, uint8_t value)
{
	int port,pin;
	if(sw > (SWITCH_NUM - 1)){
		return ;
	}
	port = relay_sw_table[sw].port;
	pin = relay_sw_table[sw].pin;
	printf("relay = [sw:%d,value:%d,port:%d,pin:%04x]\r\n",sw,value,port,pin);
	GPIO_WRITE_PIN(GPIO_PORT_TO_BASE(port),(1 << pin), (value > 0) ? (1 << pin) : 0);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
