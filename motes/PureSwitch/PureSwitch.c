/******************************************************************************

  Copyright (C), 2001-2011, Pure Co., Ltd.

 ******************************************************************************
  File Name     : pureX.c
  Version       : Initial Draft
  Author        : lanpinguo
  Created       : 2018/9/14
  Last Modified :
  Description   : Pure Motes node main body
  Function List :
              broadcast_recv
              PROCESS_THREAD
              PROCESS_THREAD
              PROCESS_THREAD
              recv_uc
  History       :
  1.Date        : 2018/9/14
    Author      : lanpinguo
    Modification: Created file

******************************************************************************/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/



#include "contiki.h"

#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "cfs/cfs.h"
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



#include "shell.h"
#include "serial-shell.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "i2c.h"

#include <cdc-eth.h>
#include "cdc.h"
#include <usb-api.h>
#include "usb-core.h"
#include "shell-memdebug.h"
#include "coap-server.h"
#include "coap-client.h"
/*#include "ping6.h"*/
#include "hcho-sensor.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

extern 	FUNC_DEBUG_PRINT dbg_print_csma;
extern 	FUNC_DEBUG_PRINT dbg_print_rest_engine;
extern 	FUNC_DEBUG_PRINT dbg_print_er_coap;
extern 	FUNC_DEBUG_PRINT dbg_print_ip;
extern 	FUNC_DEBUG_PRINT dbg_print_log;
extern  FUNC_DEBUG_PRINT dbg_print_er_coap_engine;
extern  FUNC_DEBUG_PRINT dbg_print_er_coap_observe_client;
extern  FUNC_DEBUG_PRINT dbg_print_coffee;
process_event_t dbg_event;


PROCESS_NAME(testcoffee_process);


/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/


PROCESS(shell_debug_process, "pure");
SHELL_COMMAND(pure_command,
	      "pure",
	      "pure [num]: blink LEDs ([num] times)",
	      &shell_debug_process);


/*---------------------------------------------------------------------------*/



PROCESS_THREAD(shell_debug_process, ev, data)
{
	char* argv[5];
	int argc;


	
	PROCESS_BEGIN();
	
	if(data == NULL) {
		PROCESS_EXIT(); 
	}
	argc = str_split((char*)data,(char*)" ",argv,5);
	/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/
	(void)argc;
	if(strncmp(argv[0], "test", 4) == 0) {

		process_start(&testcoffee_process,NULL);
	
	}
	else if(strncmp(argv[0], "default", 7) == 0){
		int wfd;
		unsigned char buf[50] = "hello world";
		int r;

		wfd = cfs_open("default.json", CFS_WRITE);
		if(wfd < 0) {
			printf("\r\nopen failed fd=[%d] \r\n",wfd);	
			PROCESS_EXIT(); 
		}

		/* Write buffer. */
		r = cfs_write(wfd, buf, sizeof(buf));
		if(r < 0) {
			printf("\r\nwrite failed fd=[%d] \r\n",wfd);	
			PROCESS_EXIT(); 
		}

		cfs_close(wfd);

	}





	
	PROCESS_END();
}




PROCESS(shell_list_neighbor_process, "list rpl neighbor");
SHELL_COMMAND(list_neighbor_command,
	      "lsnb",
	      "lsnb: list rpl neighbors",
	      &shell_list_neighbor_process);


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(shell_list_neighbor_process, ev, data)
{


	PROCESS_BEGIN();

	PROCESS_PAUSE();
	uip_ds6_nbr_dump();
	rpl_print_neighbor_list();	
	
	PROCESS_END();
}


/*---------------------------------------------------------------------------*/

PROCESS(shell_dbg_switch_process, "debug switch");
SHELL_COMMAND(dbg_sw_command,
		"debug",
		"debug [enable|disable] [module]: turn on/off the debug info of module",
		&shell_dbg_switch_process);

PROCESS_THREAD(shell_dbg_switch_process, ev, data)
{
	static int mod_id;
	const char *nextptr;
	
	
	PROCESS_BEGIN();
	
	if(data != NULL) {
	  mod_id = shell_strtolong(data, &nextptr);
	  if(nextptr != data) {
	  }
	}
	switch(mod_id){
		case 1:
			if(dbg_print_net){
				dbg_print_net = NULL;
			}
			else{
				dbg_print_net = printf;
			}
			break;
		case 2:
			if(dbg_print_csma){
				dbg_print_csma = NULL;
			}
			else{
				dbg_print_csma = printf;
			}
			break;
		case 3:
			if(dbg_print_ip){
				(dbg_print_ip) = NULL;
			}
			else{
				(dbg_print_ip) = printf;
			}
			break;
		case 4:
			if((dbg_print_log)){
				dbg_print_log = NULL;
			}
			else{
				dbg_print_log = printf;
			}
			break;
		case 5:
			if(dbg_print_rest_engine){
				dbg_print_rest_engine = NULL;
			}
			else{
				dbg_print_rest_engine = printf;
			}
			break;
		case 6:
			if(dbg_print_er_coap){
				dbg_print_er_coap = NULL;
			}
			else{
				dbg_print_er_coap = printf;
			}
			break;
		case 7:
			if(dbg_print_er_coap_engine){
				dbg_print_er_coap_engine = NULL;
			}
			else{
				dbg_print_er_coap_engine = printf;
			}
			break;
		case 8:
			if(dbg_print_er_coap_observe_client){
				dbg_print_er_coap_observe_client = NULL;
			}
			else{
				dbg_print_er_coap_observe_client = printf;
			}
			break;
		case 9:
			if(dbg_print_coffee){
				dbg_print_coffee = NULL;
			}
			else{
				dbg_print_coffee = printf;
			}
			break;
		default:
			break;
	}

	printf("\r\ntoggle module [%d] debug switch\r\n",mod_id);	
	
	PROCESS_END();
}



/*---------------------------------------------------------------------------*/

PROCESS(dbg_coap_client_process, "debug coap client");
SHELL_COMMAND(coap_client_command,
		"coap",
		"coap [enable|disable] [mode]: coap client debug ",
		&dbg_coap_client_process);


PROCESS_THREAD(dbg_coap_client_process, ev, data)
{
	//const char *nextptr;
	char* argv[5];
	int argc;
	static COAP_CLIENT_ARG_t coap_args;
	
	PROCESS_BEGIN();
	
	if(data != NULL) {
		argc = str_split((char*)data,(char*)" ",argv,5);
		/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/

		if(strncmp(argv[0], "sw", 2) == 0) {
			if(argc == 3){
				coap_args.mod_id = COAP_CLIENT_SW;

				coap_args.coap_conf = atoi(argv[1]);
				
				if(strncmp(argv[2], "on", 2) == 0) {
					coap_args.coap_param = 1;
				} 
				else if(strncmp(argv[2], "off", 3) == 0) {
					coap_args.coap_param = 0;
				}
				else{
					goto ERROR;
				}
			}
			else{
				goto ERROR;
			}
			
			
		} 
		else if(strncmp(argv[0], "res", 3) == 0) {
			coap_args.mod_id = COAP_CLIENT_OWN;
		}else{
			goto ERROR;
		}

		/*post to coap client*/
		process_post(&coap_client_process, dbg_event, &coap_args);

	}
	goto DONE;
	
ERROR:
	printf("\r\nWrong param\r\n");
    PROCESS_EXIT();

DONE:	
	printf("\r\nSuccessfully\r\n");/*dummp avoid compiler error*/
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/


void
shell_pure_init(void)
{
  shell_register_command(&pure_command);
  shell_register_command(&list_neighbor_command);
  shell_register_command(&dbg_sw_command);
  shell_register_command(&coap_client_command);
}



/*---------------------------------------------------------------------------*/
/* We first declare our processes. */
PROCESS(pure_x_shell_process, "PureX Contiki shell");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pure_x_shell_process, ev, data)
{
	PROCESS_BEGIN();
	serial_shell_init();
	shell_blink_init();
	shell_ps_init();
	shell_reboot_init();
	shell_text_init();
	shell_time_init();
	shell_memdebug_init();
	shell_pure_init();
#if COFFEE
	shell_coffee_init();
	shell_file_init();
#endif

	hcho_sensor_init(1);  
	PROCESS_END();
}





/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&pure_x_shell_process,\
	&coap_server_process, \
	&coap_client_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
