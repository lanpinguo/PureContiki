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
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \file
 *         A set of debugging macros for the netstack
 *
 * \author Nicolas Tsiftes <nvt@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Simon Duquennoy <simon.duquennoy@inria.fr>
 */

#ifndef NET_DEBUG_H
#define NET_DEBUG_H

#include "net/ip/uip.h"
#include "net/linkaddr.h"
#include <stdio.h>
typedef enum{
	CONTIKI_MOD_NONE ,			/* 00 */
	CONTIKI_MOD_SLIP_BRG,		/* 01 */
	CONTIKI_MOD_RF ,			/* 02 */
	CONTIKI_MOD_RF_RX,			/* 03 */
	CONTIKI_MOD_RF_TX,			/* 04 */
	CONTIKI_MOD_MAC ,			/* 05 */	
	CONTIKI_MOD_LINK ,			/* 06 */	
	CONTIKI_MOD_ND6,			/* 07 */
	CONTIKI_MOD_DS6_ROUTE,		/* 08 */		
	CONTIKI_MOD_RPL,			/* 09 */
	CONTIKI_MOD_RPL_OF0,		/* 10 */
	CONTIKI_MOD_RPL_ICMP6,		/* 11 */
	CONTIKI_MOD_RPL_DAG,		/* 12 */
	CONTIKI_MOD_RPL_DAG_ROOT,	/* 13 */
	CONTIKI_MOD_DS6_NBR,		/* 14 */
	CONTIKI_MOD_SICS,			/* 15 */
	CONTIKI_MOD_NET ,			/* 16 */
	CONTIKI_MOD_IP6,			/* 17 */
	CONTIKI_MOD_TCP,			/* 18 */
	CONTIKI_MOD_TSCH,			/* 19 */
	CONTIKI_MOD_TSCH_SLOT,		/* 20 */
	CONTIKI_MOD_TSCH_LOG,		/* 21 */
	CONTIKI_MOD_TSCH_QUEUE,		/* 22 */
	CONTIKI_MOD_CCM_STAR,		/* 23 */
	CONTIKI_MOD_PROCESS,		/* 24 */
	CONTIKI_MOD_OTA,			/* 25 */
	CONTIKI_MOD_CFS_COFFEE,		/* 26 */
	CONTIKI_MOD_COMMON,			/* 27 */

	CONTIKI_MOD_END,
}CONTIKI_MOD_ID_e;
	
typedef int (*TRACE_DEBUG_FILTER)(int mod, int line);

void net_debug_lladdr_print(const uip_lladdr_t *addr);
int trace_dbg_print(int mod, int level, const char * format,...);
int trace_print_filter_set(int enable, uint32_t *mod, int line_start, int line__end);
void trace_output_terminal_set(void* func);

#define DEBUG_NONE      0
#define DEBUG_PRINT     1
#define DEBUG_ANNOTATE  2
#define DEBUG_FULL      DEBUG_ANNOTATE | DEBUG_PRINT

#ifndef MODULE_ID
#define MODULE_ID	CONTIKI_MOD_NONE	
#endif

/* PRINTA will always print if the debug routines are called directly */
#ifdef __AVR__
#include <avr/pgmspace.h>
#define PRINTA(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTA(...) trace_dbg_print(MODULE_ID,__LINE__,__VA_ARGS__)
#endif

#if (DEBUG) & DEBUG_ANNOTATE
#ifdef __AVR__
#define ANNOTATE(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define ANNOTATE(...) trace_dbg_print(MODULE_ID,__LINE__,__VA_ARGS__)
#endif
#else
#define ANNOTATE(...)
#endif /* (DEBUG) & DEBUG_ANNOTATE */

#if (DEBUG) & DEBUG_PRINT
#ifdef __AVR__
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...) trace_dbg_print(MODULE_ID,__LINE__,__VA_ARGS__)
#endif
#define PRINTLLADDR(lladdr) net_debug_lladdr_print(lladdr)
#else
#define PRINTF(...)
#define PRINTLLADDR(lladdr)
#endif /* (DEBUG) & DEBUG_PRINT */

#endif /* NET_DEBUG_H */
