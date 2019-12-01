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
/*#include "ping6.h"*/


#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define UDP_EXAMPLE_ID  190

static struct uip_udp_conn *server_conn;



/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\r\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server process");

PROCESS(shell_debug_process, "pure");
SHELL_COMMAND(pure_command,
	      "pure",
	      "pure [num]: blink LEDs ([num] times)",
	      &shell_debug_process);

PROCESS(shell_list_neighbor_process, "list rpl neighbor");
SHELL_COMMAND(list_neighbor_command,
	      "lsnb",
	      "lsnb: list rpl neighbors",
	      &shell_list_neighbor_process);



/*---------------------------------------------------------------------------*/
#define MAX_PAYLOAD_LEN		30
static int seq_id;
static int reply;

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

static void
send_packet(void *ptr)
{
  char buf[MAX_PAYLOAD_LEN];

#ifdef SERVER_REPLY
  uint8_t num_used = 0;
  uip_ds6_nbr_t *nbr;

  nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL) {
    nbr = nbr_table_next(ds6_neighbors, nbr);
    num_used++;
  }

  if(seq_id > 0) {
    ANNOTATE("#A r=%d/%d,color=%s,n=%d %d\r\n", reply, seq_id,
             reply == seq_id ? "GREEN" : "RED", uip_ds6_route_num_routes(), num_used);
  }
#endif /* SERVER_REPLY */

  seq_id++;
  PRINTF("DATA send to %d 'Hello %d'\r\n",
         server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1], seq_id);
  sprintf(buf, "Hello %d from the client", seq_id);
  uip_udp_packet_sendto(client_conn, buf, strlen(buf),
                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
}

PROCESS_THREAD(shell_debug_process, ev, data)
{


	PROCESS_BEGIN();

	PROCESS_PAUSE();

	/*set_global_address();*/

	PRINTF("UDP client process started nbr:%d routes:%d\r\n",
	     NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

	print_local_addresses();

	/* new connection with remote host */
	client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL); 
	if(client_conn == NULL) {
		PRINTF("No UDP connection available, exiting the process!\r\n");
		PROCESS_EXIT();
	}
	udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT)); 

	PRINTF("Created a connection with the server ");
	PRINT6ADDR(&client_conn->ripaddr);
	PRINTF(" local/remote port %u/%u\r\n",
			UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

	send_packet(NULL);
	
	PROCESS_END();
}
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

extern 	FUNC_DEBUG_PRINT dbg_print_csma;

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
		default:
			break;
	}

	printf("\r\ntoggle module [%d] debug switch\r\n",mod_id);	
	
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/



void
shell_pure_init(void)
{
  shell_register_command(&pure_command);
  shell_register_command(&list_neighbor_command);
  shell_register_command(&dbg_sw_command);
}



/*---------------------------------------------------------------------------*/
/* We first declare our processes. */
PROCESS(pure_x_shell_process, "PureX Contiki shell");
PROCESS(humidity_sensor_process, "Temp & Humidity process");

/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&pure_x_shell_process,\
	&humidity_sensor_process,\
	&udp_server_process);

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
  
  PROCESS_END();
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

PROCESS_THREAD(humidity_sensor_process, ev, data)
{
	static uint8_t rc = 0;
	//static struct etimer et;


	PROCESS_BEGIN();

	rc += 1;
	//etimer_set(&et, CLOCK_SECOND * 2);
	printf("\r\nThis is OTA tester (%x)\r\n",rc);



	//EnterNvmApplication(0,5);

	//PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *appdata;

  if(uip_newdata()) {
    appdata = (char *)uip_appdata;
    appdata[uip_datalen()] = 0;
    PRINTF("DATA recv '%s' from ", appdata);
    PRINTF("%d",
           UIP_IP_BUF->srcipaddr.u8[sizeof(UIP_IP_BUF->srcipaddr.u8) - 1]);
    PRINTF("\r\n");
#if SERVER_REPLY
    PRINTF("DATA sending reply\r\n");
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
    uip_create_unspecified(&server_conn->ripaddr);
#endif
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("\r\nUDP server started. nbr:%d routes:%d \r\n",
         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoWPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the
 * 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::. At present Wireshark copies Context/128 and
 * then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit
 * compressed address of fd00::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct
 * uncompressed addresses.
 */
 
#if 0
/* Mode 1 - 64 bits inline */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
#elif 1
/* Mode 2 - 16 bits inline */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
#else
/* Mode 3 - derived from link local (MAC) address */
  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
#endif

  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\r\n");
  } else {
    PRINTF("failed to create a new RPL DAG\r\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\r\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\r\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\r\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
