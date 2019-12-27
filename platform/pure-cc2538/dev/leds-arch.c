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
 * \addtogroup cc2538-smartrf
 * @{
 *
 * \defgroup cc2538dk-leds SmartRF06EB LED driver
 *
 * LED driver implementation for the TI SmartRF06EB + cc2538EM
 * @{
 *
 * \file
 * LED driver implementation for the TI SmartRF06EB + cc2538EM
 */
#include "contiki.h"
#include "reg.h"
#include "dev/leds.h"
#include "dev/gpio.h"
#include "dev/ioc.h"

#define LEDS_GPIO_PIN_MASK   LEDS_ALL

typedef struct PLAT_LEDS_S
{
	uint8_t		port;
	uint16_t 	pin;

}PLAT_LEDS_t;

#define LEDS_NUM 	(sizeof(leds_table)/sizeof(PLAT_LEDS_t))

PLAT_LEDS_t leds_table[] = PLATFORM_LEDS_MAP;




/*---------------------------------------------------------------------------*/
void
leds_arch_init(void)
{
	int i;

	

	for(i = 0 ; i < LEDS_NUM; i++){
		/* Software controlled */
		GPIO_SOFTWARE_CONTROL(GPIO_PORT_TO_BASE(leds_table[i].port), (1<<leds_table[i].pin));
		
		GPIO_SET_OUTPUT(GPIO_PORT_TO_BASE(leds_table[i].port), (1<<leds_table[i].pin));
	
	    ioc_set_over(leds_table[i].port, leds_table[i].pin, IOC_OVERRIDE_OE);
	}


	printf("leds_arch_init done\r\n");
}
/*---------------------------------------------------------------------------*/
unsigned char
leds_arch_get(void)
{
  return GPIO_READ_PIN(GPIO_C_BASE, LEDS_GPIO_PIN_MASK);
}
/*---------------------------------------------------------------------------*/
void
leds_arch_set(unsigned char leds)
{
  GPIO_WRITE_PIN(GPIO_C_BASE, LEDS_GPIO_PIN_MASK, leds);
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
