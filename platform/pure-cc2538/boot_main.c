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
#include "flash.h"
#include "rom-util.h"
#include "xmem.h"
#include "ota_types.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/

enum {
	ENCODING_TYPE_RAW,
	ENCODING_TYPE_UTF8,
};
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#ifndef BOOT_VERSION_STRING
#define BOOT_VERSION_STRING "1.0.x"
#endif /* BOOT_VERSION_STRING */

#define SHARED_BUF_MAX						(2 * 1024)

#define GPIO_A_DIR							0x400D9400
#define GPIO_A_DATA							0x400D9000
#define GPIO_C_DIR							0x400DB400
#define GPIO_C_DATA							0x400DB000

#define HWREG(x)  							(*((volatile unsigned long *)(x)))



static char dummy[] 		= "dummy loop\r\n";
static char no_img_found[] = "No Image Found\r\n";
static char img_found[] 	= "Found Image:\r\n";


static char shared_buf[SHARED_BUF_MAX] ;
/*---------------------------------------------------------------------------*/
void uart_write_byte(uint8_t uart, uint8_t b);
uint32_t W25qxx_ReadID(void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/



void EnterNvmApplication(uint32_t spInit, uint32_t resetVector)
{
	// Set the LR register to a value causing a processor fault in case of
	// an accidental return from the application.
	asm("mov r2, #0xffffffff");
	asm("mov lr, r2");

	asm("mov sp, r0");	// Setup the initial stack pointer value.

	asm("bx r1");  // Branch to application reset ISR.
}



int dbg_output(char * buf, uint32_t encoding, uint32_t len)
{
	int i = 0;

	for(i = 0; i < len ; i++){
		char raw_out = buf[i];
		if(encoding == ENCODING_TYPE_UTF8){
			char out;
			
			for(int j = 0; j < 2; j++, raw_out <<= 4){
				
				out = (raw_out & 0xF0) >> 4;
				
				if(out < 10){
					out = out + '0';
				}
				else{
					out = out + ('A' - 10);
				}
				uart_write_byte(0, out);
			}
		}
		else{
			uart_write_byte(0, raw_out);
		}
	}
	return i;
}

int boot_app(void)
{
	volatile uint32_t ledgerPageAddr = FLASH_FW_ADDR;

	// Set direction output and initial value for PC2 and PC0
	// Greed LED on PA2
	// Red LED on PA4
	// Blue LED on PA5
	HWREG(GPIO_A_DIR) |= 0x34;
	HWREG(GPIO_A_DATA + (0x34 << 2)) = 0;
	
	//xmem_init();

	

	for (int pgCnt = 0; pgCnt < HAL_IBM_LEDGER_PAGE; pgCnt++, ledgerPageAddr += FLASH_PAGE_SIZE)
	{
		OTA_ImageHeader_t *pLedger = (OTA_ImageHeader_t *)(ledgerPageAddr + NVIC_VECTOR_SIZE);

		if (pLedger->magicNumber != OTA_HDR_MAGIC_NUMBER)
		{
			continue;
		}
		// Turn on both LED's.
		HWREG(GPIO_A_DATA + (0x30 << 2)) = 0x30;

		if (pLedger->imageValid == OTA_HEADER_IMG_VALID)
		{
			dbg_output(img_found, ENCODING_TYPE_RAW, sizeof(img_found));
			dbg_output((char*)&pLedger->headerLength,ENCODING_TYPE_UTF8, 6);
			dbg_output("\r\n", ENCODING_TYPE_RAW, 2);
			dbg_output("on page:", ENCODING_TYPE_RAW, sizeof("on page:"));
			dbg_output((char*)&pgCnt, ENCODING_TYPE_UTF8, 4);
			dbg_output("\r\n", ENCODING_TYPE_RAW, 2);
			HWREG(GPIO_A_DATA + (0x04 << 2)) = 0x04;
			ibm_ledger_t* img = (ibm_ledger_t*)(ledgerPageAddr);
			// Sanity check NVIC entries.
			if ((img->stackPtr > 0x20000000) &&
					(img->stackPtr < 0x27007FFF) &&
					(img->nvicReset > 0x200000) &&
					(img->nvicReset < 0x0027EFFF))
			{
				HWREG(GPIO_A_DATA + (0x20 << 2)) = 0x00;
				EnterNvmApplication(img->stackPtr, img->nvicReset);
			}
		}
	}


	return -1;
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Main routine for the cc2538dk platform
 */

void debug_led(void)
{
	volatile unsigned long ulLoopCount;
	REG(GPIO_A_BASE + GPIO_DIR) = 0x04; /* PA2 output*/

	// Turn Blue	LED.
	REG(GPIO_A_BASE + GPIO_DATA + (0x04 << 2)) ^= 0x04;

	// Delay for a bit
	for(ulLoopCount = 200000; ulLoopCount > 0; ulLoopCount--);
}

 
int main(void)
{
	int rc = 0;
	uint32_t w25q_id;
	OTA_FlashImageHeader_t  img_hdr ;
	OTA_FlashImageStatus_t  img_status;
	uint32_t imgHeaderStartOffset = 0;
	uint32_t imgStatusOffset = 0;
	uint32_t imgDataStartOffset = 0;

		
	
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

	w25q_id = W25qxx_ReadID();

	dbg_output("Bootloader Version:", ENCODING_TYPE_RAW, sizeof("Bootloader Version:"));
	dbg_output(BOOT_VERSION_STRING, ENCODING_TYPE_RAW, sizeof(BOOT_VERSION_STRING));
	dbg_output("\r\n", ENCODING_TYPE_RAW, 2);
	dbg_output("SPI-Flash ID:", ENCODING_TYPE_RAW, sizeof("SPI-Flash ID:"));
	dbg_output((char*)&w25q_id, ENCODING_TYPE_UTF8, sizeof(w25q_id));
	dbg_output("\r\n", ENCODING_TYPE_RAW, 2);

	//watchdog_start();
	//fade(LEDS_ORANGE);


	/* Try copy a image from external flash */

	for(int s = 0; s < 2 ; s++ ){

		if(s == 0){
			imgHeaderStartOffset 	= IMG_1_HEADER_START;
			imgStatusOffset 		= IMG_1_STATUS_OFFSET;
			imgDataStartOffset 		= IMG_1_DATA_START ;
		}
		else{
			imgHeaderStartOffset 	= IMG_2_HEADER_START;
			imgStatusOffset 		= IMG_2_STATUS_OFFSET;
			imgDataStartOffset 		= IMG_2_DATA_START;
		}
	
		xmem_pread(&img_hdr,sizeof(OTA_FlashImageHeader_t),imgHeaderStartOffset);
		xmem_pread(&img_status,sizeof(OTA_FlashImageStatus_t),imgStatusOffset);
		
		dbg_output((s == 0 ? "Check ext-img 1:" : "Check ext-img 2:"),
					ENCODING_TYPE_RAW, 
					sizeof("Check ext-img 1:"));
		dbg_output((char*)&img_hdr, ENCODING_TYPE_UTF8, sizeof(img_hdr));
		dbg_output("\r\n", ENCODING_TYPE_RAW, 2);

		
		if(img_hdr.deviceType != 0xFFFFFFFF &&
			img_hdr.fileLen < FLASH_FW_SIZE){
			
			int pos;
			int count;
			int i;
			

			if(img_status.status == OTA_EXT_IMG_STATUS_STALE){
				continue;
			}
			
			dbg_output((s == 0 ? "Found ext-img-1:" : "Found ext-img-2:"),
						ENCODING_TYPE_RAW,
						sizeof("Found ext-img-1:"));
			dbg_output((char*)&img_hdr.fileLen, ENCODING_TYPE_UTF8, sizeof(img_hdr.fileLen));
			dbg_output("\r\n", ENCODING_TYPE_RAW, 2);

			/* First, erase on-chip flash */
			rom_util_page_erase(FLASH_FW_ADDR, FLASH_FW_SIZE);

			dbg_output("copy ext-img :\r\n", ENCODING_TYPE_RAW, sizeof("copy ext-img :\r\n"));
			/* copy ext-img to on-chip flash */
			for(pos = 0, i = 1 ; pos < img_hdr.fileLen; i++){

				if(img_hdr.fileLen - pos < SHARED_BUF_MAX){
					count = img_hdr.fileLen - pos;
					/*memset(shared_buf,0xFF,SHARED_BUF_MAX);*/
					if(count % FLASH_WORD_SIZE != 0){
						count = (count / FLASH_WORD_SIZE + 1) * FLASH_WORD_SIZE;
					}
				}
				else{
					count = SHARED_BUF_MAX;
				}
				
				xmem_pread(shared_buf, count, imgDataStartOffset + pos);

				dbg_output(".", ENCODING_TYPE_RAW, sizeof("."));
				if( i % 32 == 0){
					dbg_output("\r\n", ENCODING_TYPE_RAW, sizeof("\r\n"));
				}
				
			    INTERRUPTS_DISABLE();
				
				rom_util_program_flash((uint32_t *)shared_buf, FLASH_FW_ADDR + pos, count);
				
			    INTERRUPTS_ENABLE();
				
				
				pos += count;
			}

			dbg_output("\r\n", ENCODING_TYPE_RAW, sizeof("\r\n"));
			dbg_output("Copy Done!!\r\n", ENCODING_TYPE_RAW, sizeof("Copy Done!!\r\n"));

			/* Set flag indicating ext-image already copyed into on-chip flash */
			img_status.status = OTA_EXT_IMG_STATUS_STALE;
			xmem_pwrite(&img_status,sizeof(OTA_FlashImageStatus_t),imgStatusOffset);
			
			break;

		}
	}


	dbg_output("booting:\r\n", ENCODING_TYPE_RAW, sizeof("booting:\r\n"));
	rc = boot_app();
	if(rc != 0){
		dbg_output(no_img_found, ENCODING_TYPE_RAW, sizeof(no_img_found));
	}


	while(1) {

		dbg_output(dummy, ENCODING_TYPE_RAW, sizeof(dummy));
		debug_led();
		/* We have serviced all pending events. Enter a Low-Power mode. */
		lpm_enter();
	}
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
