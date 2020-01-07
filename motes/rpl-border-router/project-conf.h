/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

#ifndef PROJECT_ROUTER_CONF_H_
#define PROJECT_ROUTER_CONF_H_

#ifndef WITH_NON_STORING
#define WITH_NON_STORING 0 /* Set this to run with non-storing mode */
#endif /* WITH_NON_STORING */

#if WITH_NON_STORING
#undef RPL_NS_CONF_LINK_NUM
#define RPL_NS_CONF_LINK_NUM 40 /* Number of links maintained at the root */
#undef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES 0 /* No need for routes */
#undef RPL_CONF_MOP
#define RPL_CONF_MOP RPL_MOP_NON_STORING /* Mode of operation*/
#endif /* WITH_NON_STORING */

#ifndef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE rpl_interface
#endif

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM          4
#endif

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    140
#endif

#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  60
#endif

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif

/*---------------------------------------------------------------------------*/
/** \name Button configuration*/

/** BUTTON_SELECT -> PA2 */
#define BUTTON_CANCEL_PORT      GPIO_A_NUM
#define BUTTON_CANCEL_PIN       2
#define BUTTON_CANCEL_VECTOR    GPIO_A_IRQn

/** BUTTON_SELECT -> PA3 */
#define BUTTON_SELECT_PORT      GPIO_A_NUM
#define BUTTON_SELECT_PIN       3
#define BUTTON_SELECT_VECTOR    GPIO_A_IRQn

/** BUTTON_LEFT -> PC4 */
#define BUTTON_LEFT_PORT        GPIO_C_NUM
#define BUTTON_LEFT_PIN         4
#define BUTTON_LEFT_VECTOR      GPIO_C_IRQn

/** BUTTON_RIGHT -> PC5 */
#define BUTTON_RIGHT_PORT       GPIO_C_NUM
#define BUTTON_RIGHT_PIN        5
#define BUTTON_RIGHT_VECTOR     GPIO_C_IRQn

/** BUTTON_UP -> PC6 */
#define BUTTON_UP_PORT          GPIO_C_NUM
#define BUTTON_UP_PIN           6
#define BUTTON_UP_VECTOR        GPIO_C_IRQn

/** BUTTON_DOWN -> PC7 */
#define BUTTON_DOWN_PORT        GPIO_C_NUM
#define BUTTON_DOWN_PIN         7
#define BUTTON_DOWN_VECTOR      GPIO_C_IRQn

/* Notify various examples that we have Buttons */
#define PLATFORM_HAS_BUTTON     1
/** @} */

/* Notify that we have LEDs */
#define PLATFORM_HAS_LEDS       		1



#define PLATFORM_LEDS_MAP 		\
{ \
	/**                 PORT   PIN **/ \
	/** LED0 -> PC1 */ {GPIO_B_NUM, 1}, \
}

#define SLIP_ARCH_CONF_USB      1

#endif /* PROJECT_ROUTER_CONF_H_ */
