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

/**
 * \file
 *         A set of debugging tools for the IP stack
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Simon Duquennoy <simon.duquennoy@inria.fr>
 */
#include <strformat.h>

#include "net/net-debug.h"

#define MODULE_ID CONTIKI_MOD_NONE
int default_print_filter(int mod, int line);

TRACE_DEBUG_FILTER dbg_print_filter = default_print_filter;

static int g_enable = 0;
static int g_mod_start = 0;
static int g_mod_end = 0;
static int g_line_start = 0;
static int g_line_end = 20000;

static StrFormatContext net_ctxt =
{
	NULL,
	NULL
};


/*---------------------------------------------------------------------------*/
int _trace_dbg_print(int mod, int line, const char *format, va_list ap)
{
	int     rc = 0;

	if(dbg_print_filter){
		if(dbg_print_filter(mod, line)){
			if(net_ctxt.write_str != NULL){
				rc = format_str_v(&net_ctxt, format, ap);
			}
		}
	}
	return rc;
}

int trace_dbg_print(int mod, int line, const char * format,...)
{
	va_list ap;
	int     rc = 0;

	va_start(ap, format);
	rc = _trace_dbg_print(mod,line,format,ap);
	va_end(ap);
	return rc;
}

int default_print_filter(int mod, int line)
{
	int     rc = 0;

	if(g_enable){
		if((mod >= g_mod_start) && (mod <= g_mod_end) && (line >= g_line_start) && (line <= g_line_end)){
			rc = 1;
		}
	}
	return rc;
}

int trace_print_filter_set(int enable,int mod_start,int mod_end,int line_start, int line__end)
{
	if(enable >= 0){
		g_enable = enable;
	}
	if(mod_start >= 0){
		g_mod_start = mod_start;
	}
	if(mod_end >= 0){
		g_mod_end = mod_end;
	}
	if(line_start >= 0){
		g_line_start = line_start;
	}
	if(line__end >= 0){
		g_line_end = line__end;
	}
	return 0;
}

void trace_output_terminal_set(void* func)
{
	net_ctxt.write_str = func;
}

/*---------------------------------------------------------------------------*/
void
net_debug_lladdr_print(const uip_lladdr_t *addr)
{
  if(addr == NULL) {
    PRINTA("(NULL LL addr)");
    return;
  } else {
#if NETSTACK_CONF_WITH_RIME
    /* Rime uses traditionally a %u.%u format */
    PRINTA("%u.%u", addr->addr[0], addr->addr[1]);
#else /* NETSTACK_CONF_WITH_RIME */
    unsigned int i;
    for(i = 0; i < LINKADDR_SIZE; i++) {
      if(i > 0) {
        PRINTA(":");
      }
      PRINTA("%02x", addr->addr[i]);
    }
#endif /* NETSTACK_CONF_WITH_RIME */
  }
}
/*---------------------------------------------------------------------------*/
