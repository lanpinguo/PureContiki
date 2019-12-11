/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *         Shell commands for memory debugging
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "contiki.h"
#include "shell-memdebug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
PROCESS(shell_poke_process, "poke");
SHELL_COMMAND(poke_command,
	      "poke",
	      "poke <address> <bytes>: write 4 bytes <bytes> to address <address>",
	      &shell_poke_process);
PROCESS(shell_peek_process, "peek");
SHELL_COMMAND(peek_command,
	      "peek",
	      "peek <address>: read 4 bytes from address <address>",
	      &shell_peek_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_poke_process, ev, data)
{
	uint32_t *address;
	uint32_t bytes;
	char* argv[3];
	int argc;

	PROCESS_BEGIN();

	if(data == NULL) {
		shell_output_str(&poke_command, "usage 0", "");
		PROCESS_EXIT();
	}

	argc = str_split((char*)data,(char*)" ",argv,3);
  
	if(argc != 2) {
		shell_output_str(&poke_command, "usage 1", "");
		PROCESS_EXIT();
	}
	
	address = (uint32_t *)strtoul((const char*)argv[0],NULL, 16);

	bytes = strtoul((const char*)argv[1],NULL, 16);

	printf("address[%p] = 0x%08x\n", address, (unsigned int)bytes);
  
	(*((volatile unsigned long *)(address))) = bytes;

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_peek_process, ev, data)
{
	uint32_t *address;
	char* argv[3];
	int argc;
	char buf[32];

	PROCESS_BEGIN();

	if(data == NULL) {
		shell_output_str(&peek_command, "usage 0", "");
		PROCESS_EXIT();
	}
	argc = str_split((char*)data,(char*)" ",argv,3);
  
	if(argc != 1) {
		shell_output_str(&poke_command, "usage 1", "");
		PROCESS_EXIT();
	}
	
	address = (uint32_t *)(uintptr_t)strtoul((const char*)argv[0],NULL, 16);

	snprintf(buf, sizeof(buf), "[%p] = 0x%08x",address, (unsigned int)(*((volatile unsigned long *)(address))));

	shell_output_str(&peek_command, buf, "");

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_memdebug_init(void)
{
  shell_register_command(&poke_command);
  shell_register_command(&peek_command);
}
/*---------------------------------------------------------------------------*/
