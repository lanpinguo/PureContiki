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
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/cc2538-rf.h"
#include "dev/udma.h"
#include "dev/crypto.h"
#include "usb/usb-serial.h"
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
#include "util.h"

#if USB_ETH_CONF_ENABLE
#include <cdc-eth.h>
#endif

#include <contikimac.h>

#if PLATFORM_HAS_RELAY_SWITCH
#include "relay_switch.h"
#endif

#include "xmem.h"


#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_COMMON
#include "net/ip/uip-debug.h"

/*---------------------------------------------------------------------------*/

#if UART_CONF_ENABLE
#define PUTS(s) puts(s)
#else
#define PUTS(s)
#endif

uint32_t W25qxx_ReadID(void);

/*---------------------------------------------------------------------------*/

char shell_prompt_text[] = "Mote>";

/*---------------------------------------------------------------------------*/
void
log_message(char *m1, char *m2)
{
	PRINTF("\r\n%s%s", m1, m2);
}
/*---------------------------------------------------------------------------*/
void
uip_log(char *m)
{
	PRINTF("\r\nuIP: '%s'", m);
}

#if 0
/*---------------------------------------------------------------------------*/
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      asm("nop");
    }
  }
}
#endif

/*---------------------------------------------------------------------------*/
static void
set_rf_params(void)
{
  uint16_t short_addr;
  uint8_t ext_addr[8];

  ieee_addr_cpy_to(ext_addr, 8);

  short_addr = ext_addr[7];
  short_addr |= ext_addr[6] << 8;

  /* Populate linkaddr_node_addr. Maintain endianness */
  memcpy(&linkaddr_node_addr, &ext_addr[8 - LINKADDR_SIZE], LINKADDR_SIZE);


  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, IEEE802154_PANID);
  NETSTACK_RADIO.set_value(RADIO_PARAM_16BIT_ADDR, short_addr);
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CC2538_RF_CHANNEL);
  NETSTACK_RADIO.set_object(RADIO_PARAM_64BIT_ADDR, ext_addr, 8);
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

#define CC2592_PA_EN_PIN      (1<<3) /*PC3*/
#define CC2592_LNA_EN_PIN     (1<<2) /*PC2*/
#define CC2592_HGM_PIN        (1<<2) /*PD2*/

int cc2592_init(void)
{
	PRINTF("\r\ncc2592 init\r\n");
	/*REG(GPIO_C_BASE + GPIO_DIR) |= (CC2592_PA_EN_PIN | CC2592_LNA_EN_PIN);*/
	REG(GPIO_D_BASE + GPIO_DIR) |= CC2592_HGM_PIN; 
	return 0;
}


/** Enable Rx. */
int cc2592_rx_enable(void)
{
	//PRINTF("\r\ncc2592_rx_enable\r\n");
	REG(GPIO_C_BASE + GPIO_DATA + (CC2592_LNA_EN_PIN << 2)) = CC2592_LNA_EN_PIN;
	return 0;
}

/** Enable Tx. */
int cc2592_tx_enable(void)
{
	//PRINTF("\r\ncc2592_tx_enable\r\n");
	REG(GPIO_C_BASE + GPIO_DATA + (CC2592_LNA_EN_PIN << 2)) = ~CC2592_LNA_EN_PIN;
	REG(GPIO_C_BASE + GPIO_DATA + (CC2592_PA_EN_PIN << 2)) = CC2592_PA_EN_PIN;
	return 0;
}

/** Turn the radio on. */
int cc2592_hgm_enable(void)
{
	//PRINTF("\r\ncc2592_hgm_enable\r\n");
	REG(GPIO_D_BASE + GPIO_DATA + (CC2592_HGM_PIN << 2)) = CC2592_HGM_PIN;
	return 0;
}

/** Turn the extender off. */
int cc2592_off(void)
{
	//PRINTF("\r\ncc2592_off\r\n");
	REG(GPIO_C_BASE + GPIO_DATA + (CC2592_LNA_EN_PIN << 2)) = ~CC2592_LNA_EN_PIN;
	REG(GPIO_C_BASE + GPIO_DATA + (CC2592_PA_EN_PIN << 2)) = ~CC2592_PA_EN_PIN;
	return 0;
}

int rf_ext_driver_register(void)
{
    cc2592_init();
    cc2592_hgm_enable();
	return 0;
}


int show_system_info(uint32_t mode)
{
    int i;

	if(mode & 0x1){
		PUTS(CONTIKI_VERSION_STRING);
		PUTS(BOARD_STRING);

		soc_print_info();

		printf("\r\nSys Status: 0x%08lx\r\n", REG(SYS_CTRL_CLOCK_STA));
		printf("Net: ");
		printf("%s\r\n", NETSTACK_NETWORK.name);
		printf("MAC: ");
		printf("%s\r\n", NETSTACK_MAC.name);
		printf("RDC: ");
		printf("%s\r\n", NETSTACK_RDC.name);
		printf("ex-flash:%08lx\r\n", W25qxx_ReadID());

	}
	
	if(mode & 0x02){
	    printf("Node address ");
	    for(i = 0; i < LINKADDR_SIZE - 1; i++) {
	      printf("%02x:", linkaddr_node_addr.u8[i]);
	    }
	    printf("%02x\r\n", linkaddr_node_addr.u8[i]);
	}
	return 0;
}

 
int
main(void)
{
	nvic_init();

	ioc_init();
	sys_ctrl_init();
	clock_init();
	lpm_init();
	rtimer_init();
	gpio_init();

#if PLATFORM_HAS_LEDS	
	//debug_led();
	leds_init();

	//fade(LEDS_YELLOW);
#endif


	process_init();

	watchdog_init();

#if PLATFORM_HAS_BUTTON
	button_sensor_init();
#endif
	/*
	* Character I/O Initialisation.
	* When the UART receives a character it will call serial_line_input_byte to
	* notify the core. The same applies for the USB driver.
	*
	* If slip-arch is also linked in afterwards (e.g. if we are a border router)
	* it will overwrite one of the two peripheral input callbacks. Characters
	* received over the relevant peripheral will be handled by
	* slip_input_byte instead
	*/
#if UART_CONF_ENABLE
	uart_init(0);
	uart_set_input(SERIAL_LINE_CONF_UART, serial_line_input_byte);
	serial_line_init();
#endif


#if USB_SERIAL_CONF_ENABLE
	usb_serial_init();
	usb_serial_set_input(serial_line_input_byte);
#endif

#if USB_ETH_CONF_ENABLE
	usb_cdc_eth_setup();
#endif


	INTERRUPTS_ENABLE();

	xmem_init();
	
	log_system_init();

	
	/* Initialise the H/W RNG engine. */
	random_init(0);

	udma_init();

	process_start(&etimer_process, NULL);
	ctimer_init();

	rf_ext_driver_register();


	set_rf_params();

	show_system_info(0x3);
	
#if CRYPTO_CONF_INIT
	crypto_init();
	crypto_disable();
#endif

	netstack_init();

#if NETSTACK_CONF_WITH_IPV6
	memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
	queuebuf_init();
	process_start(&tcpip_process, NULL);
#endif /* NETSTACK_CONF_WITH_IPV6 */

	adc_init();




	process_start(&sensors_process, NULL);

	energest_init();
	ENERGEST_ON(ENERGEST_TYPE_CPU);

	//mac_sniffer_callback = usbeth_send;
#if PLATFORM_HAS_LEDS
	leds_arch_init();
#endif

#if PLATFORM_HAS_RELAY_SWITCH
	relay_switch_init();
#endif


	autostart_start(autostart_processes);

	watchdog_start();
	//fade(LEDS_ORANGE);

	/*cc2538_rf_set_promiscous_mode(1);*/


	while(1) {
		uint8_t r;
		do {
			/* Reset watchdog and handle polls and events */
			watchdog_periodic();

			r = process_run();
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
