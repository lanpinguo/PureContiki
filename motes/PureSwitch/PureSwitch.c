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
#include "crc32.h"

#include "xmem.h"

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
#include "hcho-sensor.h"

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_NONE
#include "net/ip/uip-debug.h"
#include "relay_switch.h"
#include "dev/leds.h"


#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF       ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])

#define RELAY_SW_ACTIVE_TIME	(60 * 60 * 6) /* 6 hours */
#define RELAY_SW_DEACTIVE_TIME	(60 * 60 * 18) /* 18 hours */

#define UDP_SERVER_PORT 		5678
#define UDP_EXAMPLE_ID  		190

static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *tx_conn;


process_event_t dbg_event;

/*
static struct etimer et;
static uint32_t sw_on_flag = 1;
*/

PROCESS_NAME(testcoffee_process);
#if MAC_USING_TSCH
PROCESS_NAME(node_process);
#endif


extern int show_system_info(uint32_t mode);


/*---------------------------------------------------------------------------*/


PROCESS(shell_debug_process, "pure");
SHELL_COMMAND(pure_command,
	      "pure",
	      "pure [num]: blink LEDs ([num] times)",
	      &shell_debug_process);


/*---------------------------------------------------------------------------*/

void buffer_dump(uint8_t * buf, uint32_t len)
{
  uint16_t i;
  printf("block  %lu:", len);
  for (i = 0; i < len ; i++) {
    uint8_t data = buf[i];
	if(i % 16 == 0){
      printf("\r\n");
	}
    printf("%02x ", data);
  }
  printf("\r\n");
}


PROCESS_THREAD(shell_debug_process, ev, data)
{
	char* argv[5];
	int argc;
	static int i = 0;
	char buf[64];
	static	uint32_t addr,size;
	int rc;

	
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
	else if(strncmp(argv[0], "info", 4) == 0){
		show_system_info(3);
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
	else if(strncmp(argv[0], "flash", 5) == 0){
		/* Erase 3 sectors */
		//xmem_erase(6 * 512, 0);
		/* Erase external flash pages */
		xmem_erase(6 * 1024, 0);

		for(i = 1 ; i < 6 * 512; i++){
			buf[0] = 0x55;
			xmem_pwrite(buf, 1, i - 1);
		}

		memset(buf,0,64);
		for(i = 0 ; i < 6 * 512; ){
			if(6 * 512 - i < 64){
				xmem_pread(buf, size - i, i );
				buffer_dump((uint8_t*)buf, size - i);
				i += (6 * 512 -i);
				if(6 * 512 - i <= 0){
					break;
				}
			}
			else{
				xmem_pread(buf, 64, i );
				buffer_dump((uint8_t*)buf,64);
				i += 64;
			}
		}

	}

	else if(strncmp(argv[0], "erase", 5) == 0){
		if(argc == 3){
			sscanf(argv[1],"%lu", &addr);
			sscanf(argv[2],"%lu", &size);
		}
		/* Erase external flash pages */
		rc = xmem_erase(size * XMEM_ERASE_UNIT_SIZE, addr * XMEM_ERASE_UNIT_SIZE);
		if(rc < 0){
			printf("\r\nxmem erase failed");	
		}
		else{
			printf("\r\nxmem erase %u bytes from address 0x%lx done \r\n",
					rc, addr*1024);	
		}

	}
	else if(strncmp(argv[0], "dump", 4) == 0){
		
		if(argc == 3){
			sscanf(argv[1],"%lu", &addr);
			sscanf(argv[2],"%lu", &size);
		}
		printf("\r\ndump at addr 0x%lx, size %lu bytes:  \r\n",addr,size);
		for(i = 0 ; i < size;  ){
			if(size - i < 64){
				xmem_pread(buf, size - i, i );
				buffer_dump((uint8_t*)buf, size - i);
				i += (size -i);
			}
			else{
				xmem_pread(buf, 64, i );
				buffer_dump((uint8_t*)buf,64);
				i += 64;
			}
		}


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


#define BTN_4_SW 	((1 << 0) | (1 << 2) | (1 << 3))

void sw_ctrl(uint32_t active)
{
	int i;
	uint32_t sw_mask = 0;
	uint32_t sw_state = 0;

	
	if(active){
		sw_mask = BTN_4_SW;
		sw_state = BTN_4_SW;
	}else{
		sw_mask = BTN_4_SW;
		sw_state = ~BTN_4_SW;
	}
	
	for(i = 0; i < sizeof(sw_state) * 8; i++){
		if(sw_mask & (1 << i)){
			printf("set Relay-SW[%d] : %s\r\n",i,(sw_state & sw_mask) > 0 ? "on" : "off");
			relay_switch_set(i,(sw_state & sw_mask));
		}
	}



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

	/* Defalut disable status indication leds*/
	leds_arch_set(0x0);

	
#if 0
	etimer_set(&et, RELAY_SW_ACTIVE_TIME * CLOCK_SECOND);
	sw_ctrl(sw_on_flag);


	while(1){
		PROCESS_YIELD();

		if(etimer_expired(&et)) {

			if(sw_on_flag){
				etimer_set(&et, RELAY_SW_DEACTIVE_TIME * CLOCK_SECOND);
				sw_on_flag = 0;
			}
			else{
				etimer_set(&et, RELAY_SW_ACTIVE_TIME * CLOCK_SECOND);
				sw_on_flag = 1;
			}

			sw_ctrl(sw_on_flag);
			printf("sw_on_flag : %ld\r\n",sw_on_flag);


		
		}

		
  }
#endif

	PROCESS_END();
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "ota_types.h"

static OTA_Info_t ota_info;
static OTA_Info_t ota_info_current = {
	.deviceType = 1,
	.version = 0x10000,
	.primary = 1,
	.seqno = 0,
	.totalLen = 0,
	.state = OTA_STATE_NONE,
};

int32_t OTA_UpgradeStart(char * data)
{
	OTA_DataRequestFrame_t req;

	OTA_UpgradeRequestFrameHeader_t * frame = (OTA_UpgradeRequestFrameHeader_t *)data;

	show_system_info(2);
	printf("device type: %lu, version: %lu\r\n",frame->deviceType, frame->version);
	buffer_dump((uint8_t *)data,uip_datalen());


	req.type = OTA_FRAME_TYPE_DATA_REQUEST;
	req.deviceType = ota_info_current.deviceType;
	req.version = ota_info_current.version;
	req.seqno = ota_info_current.seqno = 0;
    uip_ipaddr_copy(&tx_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
	tx_conn->rport = UIP_UDP_BUF->srcport;
	uip_udp_packet_send(tx_conn, &req, sizeof(req));
	ota_info_current.state = OTA_STATE_RUNNING;

	return 0;

}


int32_t OTA_StateMachineUpdate(char* data, uint32_t event)
{
	uint32_t pkt_type = OTA_FRAME_TYPE_NONE;



	if(data != NULL){
		pkt_type = data[0];
	}

	
	switch(ota_info_current.state){
		case OTA_STATE_NONE:{
			if(pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST){
				OTA_UpgradeStart(data);

				/* Erase external flash pages */
				//xmem_erase(516 * 1024, 0);
			}
			break;
		}
		case OTA_STATE_START:{


			break;
		}

		case OTA_STATE_RUNNING:{
			if(pkt_type == OTA_FRAME_TYPE_DATA ||
				pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST ||
				pkt_type == OTA_FRAME_TYPE_FINISH ){
				
				OTA_DataRequestFrame_t req;
				OTA_DataFrameHeader_t * frame = (OTA_DataFrameHeader_t *)data;

				if (pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST){
					OTA_UpgradeRequestFrameHeader_t * frame = (OTA_UpgradeRequestFrameHeader_t *)data;

					if(frame->option == OTA_UPGRADE_OPTION_RESTART){
						OTA_UpgradeStart(data);
					}

				}
				
				printf("seqno %u, data len: %u\r\n",frame->seqno, frame->dataLength);
				//buffer_dump((uint8_t *)data,uip_datalen());
				/* Write data into flash */
				xmem_pwrite(frame->data, frame->dataLength, 4 * 1024 + ota_info_current.totalLen);
				ota_info_current.totalLen += frame->dataLength;
				
				/* Request next block */
				req.type = OTA_FRAME_TYPE_DATA_REQUEST;
				req.deviceType = ota_info_current.deviceType;
				req.version = ota_info_current.version;
				req.seqno = ++ota_info_current.seqno;

				if(req.seqno == ota_info_current.maxSeqno ||
					pkt_type == OTA_FRAME_TYPE_FINISH ){
					ota_info_current.state = OTA_STATE_FINISH;
				}
				
			    uip_ipaddr_copy(&tx_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
				tx_conn->rport = UIP_UDP_BUF->srcport;
				uip_udp_packet_send(tx_conn, &req, sizeof(req));
			}
			break;
		}
		case OTA_STATE_FINISH:{
			if(pkt_type == OTA_FRAME_TYPE_FINISH){
				OTA_FinishFrameHeader_t * frame = (OTA_FinishFrameHeader_t *)data;
				uint32_t checkCode = 0xFFFFFFFF;
				uint32_t i;

				
				printf("seqno %u, checkCode: 0x%08lx\r\n",frame->seqno, frame->checkCode);
				//buffer_dump((uint8_t *)data,uip_datalen());

				/* re-generate check code */
				for(i = 0; i < ota_info_current.totalLen; ){
					uint8_t buf[1024];
					int len = 1024;

					if(ota_info_current.totalLen - i < 1024){
						len = ota_info_current.totalLen - i ;
					}
						
					xmem_pread(buf,len,i);
					
					checkCode = crc32_data(buf, len, checkCode);
					printf("local checkCode: 0x%08lx\r\n", checkCode);

					i += len;
					if(len <= 0){
						break;
					}
				}

				
				ota_info_current.state = OTA_STATE_NONE;
			}

			break;
		}
		default :
			break;

	}

	return 0;
}

static void
ota_packet_handler(void)
{
	char *appdata;

	if(uip_newdata()) {

		appdata = (char *)uip_appdata;
		appdata[uip_datalen()] = 0;

		//buffer_dump((uint8_t *)appdata,uip_datalen());
		OTA_StateMachineUpdate(appdata,0);
#if 0
		printf("DATA recv '%s' from ", appdata);
		printf("%d", UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
		printf("\r\n");

		printf("DATA sending reply\r\n");
	    uip_ipaddr_copy(&tx_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
		tx_conn->rport = UIP_UDP_BUF->srcport;
		//uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
		uip_udp_packet_send(tx_conn, "Reply", sizeof("Reply"));
		//uip_create_unspecified(&server_conn->ripaddr);
#endif		
	}
}



/*---------------------------------------------------------------------------*/
PROCESS(ota_upgrade_process, "ota-upgrade");

PROCESS_THREAD(ota_upgrade_process, ev, data)
{

	PROCESS_BEGIN();

	PROCESS_PAUSE();


	printf("\r\nUDP server started. nbr:%d routes:%d \r\n",
			NBR_TABLE_CONF_MAX_NEIGHBORS,
			UIP_CONF_MAX_ROUTES);

	//print_local_addresses();


	server_conn = udp_new(NULL, 0, NULL);
	if(server_conn == NULL) {
		printf("No UDP connection available, exiting the process!\r\n");
		PROCESS_EXIT();
	}
	
	udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

	printf("Created a server connection with remote address ");
	uip_ipaddr_print(&server_conn->ripaddr);
	printf(" local/remote port %u/%u\r\n",
			UIP_HTONS(server_conn->lport),
			UIP_HTONS(server_conn->rport));


	tx_conn = udp_new(NULL, 0, NULL);
	if(tx_conn == NULL) {
		printf("No UDP connection available, exiting the process!\r\n");
		PROCESS_EXIT();
	}


	while(1) {
		PROCESS_YIELD();
		if(ev == tcpip_event) {
			ota_packet_handler();
		}
	}

	PROCESS_END();
}




/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(
	&shell_debug_process,
	&pure_x_shell_process,
#if MAC_USING_TSCH
	&node_process,
#endif
	&ota_upgrade_process,
	&coap_server_process, 
	&coap_client_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
