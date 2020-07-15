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
 * \addtogroup cc2538-platforms
 * @{
 *
 * \defgroup cc2538dk The cc2538 Development Kit platform
 *
 * The cc2538DK is a platform by Texas Instruments, based on the
 * cc2538 SoC with an ARM Cortex-M3 core.
 * @{
 *
 * \file
 *   Main module for the cc2538dk platform
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/adc.h"
#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "dev/nvic.h"
#include "dev/uart.h"
#include "dev/watchdog.h"
#include "dev/ioc.h"
#include "dev/serial-line.h"
#include "dev/crypto.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/queuebuf.h"
#include "net/ip/tcpip.h"
#include "net/ip/uip.h"
#include "net/mac/frame802154.h"
#include "soc.h"
#include "cpu.h"
#include "reg.h"
#include "ieee-addr.h"
#include "lpm.h"


#include "xmem.h"


#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if UART_CONF_ENABLE
#define PUTS(s) puts(s)
#else
#define PUTS(s)
#endif
/*---------------------------------------------------------------------------*/

FUNC_DEBUG_PRINT dbg_print_ip = NULL;
FUNC_DEBUG_PRINT dbg_print_log = NULL;

/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
	if(dbg_print_log){ 
		dbg_print_log("\r\n%s%s", m1, m2);
	}
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
	if(dbg_print_ip){ 
		dbg_print_ip("\r\nuIP: '%s'", m);
	}
}


/*---------------------------------------------------------------------------*/
/**
 * \brief Main routine for the cc2538dk platform
 */

void debug_led(void)
{
	volatile unsigned long ulLoopCount;
	REG(GPIO_A_BASE + GPIO_DIR) = 0x04; /* PA2 output*/

	// Loop forever.
	while(1)
	{
			// Turn Blue	LED.
			REG(GPIO_A_BASE + GPIO_DATA + (0x04 << 2)) ^= 0x04;

			// Delay for a bit
			for(ulLoopCount = 200000; ulLoopCount > 0; ulLoopCount--)
			{
			}
	};
}

 
int
main(void)
{
	nvic_init();

	ioc_init();
	sys_ctrl_init();
	//clock_init();
	//lpm_init();
	//rtimer_init();
	//gpio_init();




	//watchdog_init();

	uart_init(0);



	xmem_init();




	//watchdog_start();
	//fade(LEDS_ORANGE);



	while(1) {
		uint8_t r;
		do {
			/* Reset watchdog and handle polls and events */
			//watchdog_periodic();

		} while(r > 0);

		/* We have serviced all pending events. Enter a Low-Power mode. */
		lpm_enter();
	}
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
