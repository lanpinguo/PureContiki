/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Device driver for the ST M25P80 40MHz 1Mbyte external memory.
 * \author
 *         Björn Grönvall <bg@sics.se>
 *
 *         Data is written bit inverted (~-operator) to flash so that
 *         unwritten data will read as zeros (UNIX style).
 */


#include "contiki.h"
#include <stdio.h>
#include <string.h>

#include "spi-arch.h"
#include "dev/spi.h"
#include "dev/xmem.h"
#include "dev/watchdog.h"

#include "contiki-conf.h"


/* Enable/disable flash access to the SPI bus (active low). */

#define SPI_FLASH_ENABLE()  			SPI_CS_CLR(SPI_XMEM_CS_PORT, SPI_XMEM_CS_PIN)
#define SPI_FLASH_DISABLE() 			SPI_CS_SET(SPI_XMEM_CS_PORT, SPI_XMEM_CS_PIN)

#define SPI_FLASH_HOLD()               
#define SPI_FLASH_UNHOLD()              



#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#define  SPI_FLASH_INS_WREN        0x06
#define  SPI_FLASH_INS_WRDI        0x04
#define  SPI_FLASH_INS_RDSR        0x05
#define  SPI_FLASH_INS_WRSR        0x01
#define  SPI_FLASH_INS_READ        0x03
#define  SPI_FLASH_INS_FAST_READ   0x0b
#define  SPI_FLASH_INS_PP          0x02
#define  SPI_FLASH_INS_SE          0xd8
#define  SPI_FLASH_INS_BE          0xc7
#define  SPI_FLASH_INS_DP          0xb9
#define  SPI_FLASH_INS_RES         0xab
/*---------------------------------------------------------------------------*/


uint32_t W25qxx_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	SPI_FLASH_ENABLE();

	SPI_WRITE(0x9F);

	SPI_FLUSH();
	SPI_READ(Temp0);
	SPI_READ(Temp1);
	SPI_READ(Temp2);
	
	SPI_FLASH_DISABLE();
	
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	return Temp;
}



/*---------------------------------------------------------------------------*/
static void
write_enable(void)
{

  SPI_FLASH_ENABLE();
  
  SPI_WRITE(SPI_FLASH_INS_WREN);

  SPI_FLASH_DISABLE();
}
/*---------------------------------------------------------------------------*/
static unsigned
read_status_register(void)
{
  unsigned char u;


  SPI_FLASH_ENABLE();
  
  SPI_WRITE(SPI_FLASH_INS_RDSR);

  SPI_FLUSH();
  SPI_READ(u);

  SPI_FLASH_DISABLE();

  return u;
}
/*---------------------------------------------------------------------------*/
/*
 * Wait for a write/erase operation to finish.
 */
static unsigned
wait_ready(void)
{
  unsigned u;
  do {
    u = read_status_register();
    watchdog_periodic();
  } while(u & 0x01);		/* WIP=1, write in progress */
  return u;
}
/*---------------------------------------------------------------------------*/
/*
 * Erase 64k bytes of data. It takes about 1s before WIP goes low!
 */
static void
erase_sector(unsigned long offset)
{

  wait_ready();
  write_enable();

  SPI_FLASH_ENABLE();
  
  SPI_WRITE_FAST(SPI_FLASH_INS_SE);
  SPI_WRITE_FAST(offset >> 16);	/* MSB */
  SPI_WRITE_FAST(offset >> 8);
  SPI_WRITE_FAST(offset >> 0);	/* LSB */
  SPI_WAITFORTx_ENDED();

  SPI_FLASH_DISABLE();
}
/*---------------------------------------------------------------------------*/
/*
 * Initialize external flash *and* SPI bus!
 */
void
xmem_init(void)
{
	uint32_t w25q_id;
	
	spi_init();

	/* Current spi clock is 16MHz */

	/* Mode 0 */
	spix_set_mode(SPI_DEFAULT_INSTANCE, SSI_CR0_FRF_MOTOROLA,0,0,8);
	
	spix_cs_init(SPI_XMEM_CS_PORT, SPI_XMEM_CS_PIN);

	w25q_id = W25qxx_ReadID();
	/* Release from Deep Power-down */
	SPI_FLASH_ENABLE();
	SPI_WRITE_FAST(SPI_FLASH_INS_RES);
	SPI_WAITFORTx_ENDED();
	SPI_FLASH_DISABLE();		/* Unselect flash. */

	SPI_FLASH_UNHOLD();
}
/*---------------------------------------------------------------------------*/
int
xmem_pread(void *_p, int size, unsigned long offset)
{
  unsigned char *p = _p;
  const unsigned char *end = p + size;

  wait_ready();

  ENERGEST_ON(ENERGEST_TYPE_FLASH_READ);

  SPI_FLASH_ENABLE();

  SPI_WRITE_FAST(SPI_FLASH_INS_READ);
  SPI_WRITE_FAST(offset >> 16);	/* MSB */
  SPI_WRITE_FAST(offset >> 8);
  SPI_WRITE_FAST(offset >> 0);	/* LSB */
  SPI_WAITFORTx_ENDED();
  
  SPI_FLUSH();
  for(; p < end; p++) {
    unsigned char u;
    SPI_READ(u);
    *p = ~u;
  }

  SPI_FLASH_DISABLE();

  ENERGEST_OFF(ENERGEST_TYPE_FLASH_READ);

  return size;
}
/*---------------------------------------------------------------------------*/
static const unsigned char *
program_page(unsigned long offset, const unsigned char *p, int nbytes)
{
  const unsigned char *end = p + nbytes;

  wait_ready();
  write_enable();

  SPI_FLASH_ENABLE();
  
  SPI_WRITE_FAST(SPI_FLASH_INS_PP);
  SPI_WRITE_FAST(offset >> 16);	/* MSB */
  SPI_WRITE_FAST(offset >> 8);
  SPI_WRITE_FAST(offset >> 0);	/* LSB */

  for(; p < end; p++) {
    SPI_WRITE_FAST(~*p);
  }
  SPI_WAITFORTx_ENDED();

  SPI_FLASH_DISABLE();

  return p;
}
/*---------------------------------------------------------------------------*/
int
xmem_pwrite(const void *_buf, int size, unsigned long addr)
{
  const unsigned char *p = _buf;
  const unsigned long end = addr + size;
  unsigned long i, next_page;

  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);

  for(i = addr; i < end;) {
    next_page = (i | 0xff) + 1;
    if(next_page > end) {
      next_page = end;
    }
    p = program_page(i, p, next_page - i);
    i = next_page;
  }

  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);

  return size;
}
/*---------------------------------------------------------------------------*/
int
xmem_erase(long size, unsigned long addr)
{
  unsigned long end = addr + size;

  if(size % XMEM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  if(addr % XMEM_ERASE_UNIT_SIZE != 0) {
    return -1;
  }

  for (; addr < end; addr += XMEM_ERASE_UNIT_SIZE) {
    erase_sector(addr);
  }

  return size;
}
/*---------------------------------------------------------------------------*/

