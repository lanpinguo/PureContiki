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

//#include <cdc-eth.h>
//#include "cdc.h"
//#include <usb-api.h>
//#include "usb-core.h"
#include "shell-memdebug.h"
#include "coap-server.h"
#include "coap-client.h"
/*#include "ping6.h"*/


#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

process_event_t dbg_event;


PROCESS_NAME(testcoffee_process);
#if MAC_USING_TSCH
PROCESS_NAME(node_process);
#endif
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
	static char* argv[5];
	static int argc;
	static int enable = 0;
	static int mod_start = 0;
	static int mod_end = 0;
	static int line_start = 0;
	static int line_end = 0;
	
	
	PROCESS_BEGIN();
	
	if(data != NULL) {
		argc = str_split((char*)data,(char*)" ",argv,5);
		/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/

		if(strncmp(argv[0], "line", 2) == 0) {
			if(argc == 2){
				sscanf(argv[1],"%d-%d",&line_start,&line_end);
				enable = -1;
				mod_start = -1;
				mod_end = -1;
				/*line_start = -1;*/
				/*line_end = -1;*/
			}
			else{
				goto ERROR;
			}
			
			
		} 
		else if(strncmp(argv[0], "mod", 3) == 0) {
			if(argc == 2){
				sscanf(argv[1],"%d-%d",&mod_start,&mod_end);
				enable = -1;
				/*mod_start = -1;*/
				/*mod_end = -1;*/
				line_start = -1;
				line_end = -1;
			}
			else{
				goto ERROR;
			}
		}
		else if(strncmp(argv[0], "enable", 3) == 0) {
				enable = 1;
				mod_start = -1;
				mod_end = -1;
				line_start = -1;
				line_end = -1;
		}
		else if(strncmp(argv[0], "disable", 3) == 0) {
				enable = 0;
				mod_start = -1;
				mod_end = -1;
				line_start = -1;
				line_end = -1;
		}
		else{
			goto ERROR;
		}


	}

	trace_print_filter_set(enable,mod_start,mod_end,line_start,line_end);
	goto DONE;
	
ERROR:
	printf("\r\nParameter Error !!\r\n");	
    PROCESS_EXIT();
DONE:	
	printf("\r\ntoggle module debug switch\r\n"
			"{enable:%d,mod_start:%d,mod_end:%d,line_start:%d,line_end:%d}\r\n",
			enable,mod_start,mod_end,line_start,line_end);	
	PROCESS_END();
}


void dump_config_file(void)
{
	int rfd,rc;
	int i;
	uint8_t *buf;



	rfd = cfs_open("config", CFS_READ);
	if(rfd < 0) {
		printf("\r\nopen failed fd=[%d] \r\n",rfd);	
		return;
	}

	buf = (void*)get_remote_server_address(0);

	/* Read buffer. */
	rc = cfs_read(rfd, buf, sizeof(uip_ipaddr_t) * MAX_SERVER_NUM);
	if(rc < 0) {
		printf("\r\nread failed fd=[%d] \r\n",rfd);	
	}

	printf("\r\n---------- SERVER LSIT -------------\r\n");	
	for(i = 0 ; i < MAX_SERVER_NUM; i++){
		
		uint8_t *addr = buf + sizeof(uip_ipaddr_t) * i;
		
		printf("\r\n[%d] %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", 
				i,
				addr[0], addr[1],
				addr[2], addr[3], 
				addr[4], addr[5],
				addr[6], addr[7], 
				addr[8], addr[9], 
				addr[10],addr[11], 
				addr[12],addr[13], 
				addr[14],addr[15]);

	}
	printf("\r\n---------- SERVER END -------------\r\n");	

	cfs_close(rfd);

}


/*---------------------------------------------------------------------------*/

static int32_t ip_addr_parse(void * input, uip_ipaddr_t *ipaddr)
{
	int value[10];
	int32_t rc;

	
	if(input == NULL || ipaddr == NULL){
		return -1;
	}

	rc = sscanf( input
			,"%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x"
			,&value[0],&value[1]
			,&value[2],&value[3]
			,&value[4],&value[5]
			,&value[6],&value[7]
			,&value[8],&value[9]);
	
			ipaddr->u8[0] = value[0] & 0xff;
			ipaddr->u8[1] = value[1] & 0xff;
			ipaddr->u8[8] = value[2] & 0xff;
			ipaddr->u8[9] = value[3] & 0xff;
			ipaddr->u8[10] = value[4] & 0xff;
			ipaddr->u8[11] = value[5] & 0xff;
			ipaddr->u8[12] = value[6] & 0xff;
			ipaddr->u8[13] = value[7] & 0xff;
			ipaddr->u8[14] = value[8] & 0xff;
			ipaddr->u8[15] = value[9] & 0xff;

	return rc;
}

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
		coap_args.server_id = 0;
	
		if(strncmp(argv[0], "sw", 2) == 0) {
			if(argc == 3 || argc == 4){
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

				if(argc == 4){
					coap_args.server_id = atoi(argv[3]);
				}
			}
			else{
				goto ERROR;
			}
			
			/*post to coap client*/
			process_post(&coap_client_process, dbg_event, &coap_args);
			
			
		} 
		else if(strncmp(argv[0], "res", 3) == 0) {
			coap_args.mod_id = COAP_CLIENT_OWN;
			if(argc == 2){
				coap_args.server_id = atoi(argv[1]);
			}
			/*post to coap client*/
			process_post(&coap_client_process, dbg_event, &coap_args);
		}
		else if(strncmp(argv[0], "server", 6) == 0) {
			int server_id = 0;
			static uip_ipaddr_t ipaddr;

			
			uip_ip6addr(&ipaddr, 0, 0, 0, 0, 0, 0, 0, 0);

			if(argc == 3){
				server_id = atoi(argv[1]);
				ip_addr_parse(argv[2],&ipaddr);
				printf("\r\nserver:\r\n");
				PRINT6ADDR(&ipaddr);
				set_remote_server_address(server_id, &ipaddr);

			}
			goto DONE;
		}
		else if(strncmp(argv[0], "write", 6) == 0) {
			int wfd;
			int rc;
			uint8_t *buf;

			wfd = cfs_open("config", CFS_WRITE);
			if(wfd < 0) {
				printf("\r\nopen failed fd=[%d] \r\n",wfd);	
				goto ERROR;
			}

			buf = (void*)get_remote_server_address(0);

			/* Write buffer. */
			rc = cfs_write(wfd, buf, sizeof(uip_ipaddr_t) * MAX_SERVER_NUM);
			if(rc < 0) {
				printf("\r\nwrite failed fd=[%d] \r\n",wfd);	
				goto ERROR;
			}

			cfs_close(wfd);
			

		}
		else if(strncmp(argv[0], "dump", 4) == 0) {
			dump_config_file();
		}
		else if(strncmp(argv[0], "state", 5) == 0) {
			coap_args.mod_id = COAP_CLIENT_SW_ST;
			/*post to coap client*/
			process_post(&coap_client_process, dbg_event, &coap_args);
		}
		else if(strncmp(argv[0], "hcho", 4) == 0) {
			coap_args.mod_id = COAP_CLIENT_HCHO;
			/*post to coap client*/
			process_post(&coap_client_process, dbg_event, &coap_args);
		}
		else{
			goto ERROR;
		}


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

void restore_config_from_file(){
	int rfd,rc;
	int i;
	uip_ipaddr_t buf[MAX_SERVER_NUM];


	rfd = cfs_open("config", CFS_READ);
	if(rfd < 0) {
		printf("\r\nopen failed fd=[%d] \r\n",rfd);	
		return;
	}

	/* Read buffer. */
	rc = cfs_read(rfd, buf, sizeof(uip_ipaddr_t) * MAX_SERVER_NUM);
	if(rc < 0) {
		printf("\r\nread failed fd=[%d] \r\n",rfd);	
	}

	for(i = 0 ; i < MAX_SERVER_NUM; i++){
		set_remote_server_address(i,&buf[i]);
	}



	cfs_close(rfd);

}


/*---------------------------------------------------------------------------*/
#define CRC8_POLY 0x31 //Polynomial 0x31(X8 + X5 + X4 + 1)
uint8_t CRC8(uint8_t crc, uint8_t byte)
{
  uint8_t i;
 
  crc ^= byte;
  for(i = 0; i < 8; i++)
  {
    if(crc & 0x80)
    {
      crc = (crc<< 1)^CRC8_POLY;
    }
    else
    {
      crc = crc<< 1;
    }
  }
 
  return crc;
}
 

#define MAX_RANGE (65536.0)
/*---------------------------------------------------------------------------*/
int sensors_init(void)
{
	uint8_t rc;

	i2c_init(GPIO_D_NUM,1,GPIO_D_NUM,0,I2C_SCL_NORMAL_BUS_SPEED);
	printf("\r\ni2c_init done %x \r\n",(int)(2^16));

	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset chip error (%x)\r\n",rc);
		return -1;
	}

	return 0;
}

/*---------------------------------------------------------------------------*/
float sensors_get_temperature(void)
{
	uint8_t rc;
	uint8_t buf[4];
	int tmp;
	float Temp;
	uint8_t acc;
  



	
	rc = i2c_single_send(0x40,0xe3);
	if(rc){
		printf("Trigger temp measure error(%x)\r\n",rc);
		goto error_process;
	}
	rc = i2c_burst_receive(0x40,buf,3);
	if(rc){
		printf("receive temp measure error(%x)\r\n",rc);
		goto error_process;
	}
	acc = CRC8(0,buf[0]);
	acc = CRC8(acc,buf[1]);

	if(acc != buf[2]){
		printf("Read error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[0],buf[1],buf[2],acc);
	}

	tmp = (buf[0]<<8) | (buf[1] & 0xFC);
	Temp = -46.85 + 175.72*(float)tmp/MAX_RANGE;

	goto no_error;



error_process:
	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset chip error (%x)\r\n",rc);
	}
	return 0;
no_error:

	return Temp;
}

/*---------------------------------------------------------------------------*/
float sensors_get_humidity(void)
{
	uint8_t rc;
	uint8_t buf[4];
	int tmp;
	float RH;
	uint8_t acc;
  

	rc = i2c_single_send(0x40,0xe5);
	if(rc){
		printf("Trigger RH measure error(%d)\r\n",rc);
		goto error_process;
	}
	rc = i2c_burst_receive(0x40,buf,3);
	if(rc){
		printf("receive temp measure error(%x)\r\n",rc);
		goto error_process;
	}
	acc = CRC8(0,buf[0]);
	acc = CRC8(acc,buf[1]);

	if(acc != buf[2]){
		printf("Read error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[0],buf[1],buf[2],acc);
	}

	tmp = (buf[0]<<8) | (buf[1] & 0xFC);
	RH = -6 + 125.0*((float)tmp/MAX_RANGE);


	goto no_error;



error_process:
	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset chip error (%x)\r\n",rc);
	}
	return 0;
no_error:
	return RH;
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

	/*Restore config*/
	restore_config_from_file();

	sensors_init();

	PROCESS_END();
}


/*---------------------------------------------------------------------------*/



/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&pure_x_shell_process,
#if MAC_USING_TSCH
	&node_process,
#endif
	&coap_server_process, 
	&coap_client_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
