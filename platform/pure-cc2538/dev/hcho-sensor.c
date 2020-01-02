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
#include "sys/clock.h"
#include "dev/ioc.h"
#include "dev/gpio.h"
#include "dev/adc.h"
#include "dev/hcho-sensor.h"

#include <stdint.h>




/*---------------------------------------------------------------------------*/
int
uart_line_input_byte(unsigned char c)
{
	printf("%c ",c);
	return 1;
}



/*---------------------------------------------------------------------------*/
static int
value(int type)
{

	return 0;
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

/** @} */
