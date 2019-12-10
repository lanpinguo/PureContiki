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
#include "dev/relay_switch.h"
#include "stdio.h"


#define SWITCH_GPIO_D_PIN_MASK   ((1<<LED_CTRL_PIN)|(1<<SWITCH0_PIN)|(1<<SWITCH1_PIN)|(1<<SWITCH2_PIN))
#define SWITCH_GPIO_B_PIN_MASK   ((1<<SWITCH3_PIN)|(1<<SWITCH4_PIN)|(1<<SWITCH5_PIN)|(1<<SWITCH6_PIN)|(1<<SWITCH7_PIN))


RELAY_SWITCH_t relay_sw_table[SWITCH_NUM] = {
{LED_CTRL_PORT,LED_CTRL_PIN},
{SWITCH0_PORT,SWITCH0_PIN},
{SWITCH1_PORT,SWITCH1_PIN},
{SWITCH2_PORT,SWITCH2_PIN},
{SWITCH3_PORT,SWITCH3_PIN},
{SWITCH4_PORT,SWITCH4_PIN},
{SWITCH5_PORT,SWITCH5_PIN},
{SWITCH6_PORT,SWITCH6_PIN},
{SWITCH7_PORT,SWITCH7_PIN}
};


/*---------------------------------------------------------------------------*/
void
relay_switch_init(void)
{
  /* Software controlled */
  //GPIO_SOFTWARE_CONTROL(SWITCH0_PORT, SWITCH_GPIO_D_PIN_MASK);
  //GPIO_SOFTWARE_CONTROL(SWITCH3_PORT, SWITCH_GPIO_B_PIN_MASK);
  
  GPIO_SET_OUTPUT(SWITCH0_PORT, SWITCH_GPIO_D_PIN_MASK);
  GPIO_SET_OUTPUT(SWITCH3_PORT, SWITCH_GPIO_B_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
uint8_t
relay_switch_get(uint8_t sw)
{
	if(sw > (SWITCH_NUM - 1)){
		return 0;
	}
	return GPIO_READ_PIN(relay_sw_table[sw].port, relay_sw_table[sw].pin);
}
/*---------------------------------------------------------------------------*/
void
relay_switch_set(uint8_t sw, uint8_t value)
{
	if(sw > (SWITCH_NUM - 1)){
		return ;
	}
	printf("relay = [sw:%d,value:%d,port:%d,pin:%04x]\r\n",
			sw,value,relay_sw_table[sw].port,relay_sw_table[sw].pin);
	GPIO_WRITE_PIN(relay_sw_table[sw].port, relay_sw_table[sw].pin, value);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
