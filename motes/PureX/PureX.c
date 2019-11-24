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



#define TEST  2

#if TEST == 1
static uint8_t pkt_index = 0;
static uint8_t test_data[UIP_BUFSIZE] = {
  0x14,0x75,0x90,0x73,0x55,0xb4,0x98,0x54,0x1b,0xa2,0x87,0xd0,0x08,0x00,0x45,0x00,
  0x00,0x34,0x1b,0x1c,0x40,0x00,0x40,0x06,0x4c,0x1b,0xc0,0xa8,0x02,0x64,0x0d,0xe6,
  0x02,0x9b,0xdb,0x17,0x00,0x16,0xfb,0x07,0x2b,0x89,0x00,0x00,0x00,0x00,0x80,0x02,
  0xfa,0xf0,0x9e,0xd3,0x00,0x00,0x02,0x04,0x05,0xb4,0x01,0x03,0x03,0x08,0x01,0x01,
  0x04,0x02
};

#elif TEST == 2
static uint8_t pkt_index = 0;
static uint8_t test_data[UIP_BUFSIZE] = {
  0x41, 0xd8, 0xf9, 0x34, 0x12, 0xff, 0xff, 0xf1, 0xfd, 0x05, 0x10, 0x00, 0x4b, 0x12, 0x00, 0x7a,
  0x3b, 0x3a, 0x1a, 0x9b, 0x00, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#elif TEST == 3
static uint8_t test_data[] = {
  0x02, 0x00, 0x00, 0x00, 0x00, 0x12, 0xf4, 0x8e, 0x38, 0x86, 0x3e, 0x8b, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x44, 0xc3, 0x61, 0x40, 0x00, 0x40, 0x11, 0xf0, 0x65, 0xc0, 0xa8, 0x02, 0xc9, 0xc0, 0xa8,
  0x02, 0xc8, 0x90, 0x41, 0x04, 0x00, 0x00, 0x30, 0xb4, 0x45, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x33, 0x34, 0x36, 0x37, 0x38,
  0x39, 0x30, 0x2d, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x00, 0x00, 0x00,0x00
};

#endif
/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server process");

PROCESS(shell_debug_process, "pure");
SHELL_COMMAND(pure_command,
	      "pure",
	      "pure [num]: blink LEDs ([num] times)",
	      &shell_debug_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(shell_debug_process, ev, data)
{


  PROCESS_BEGIN();

  
#if TEST == 1
  pkt_index++;
  test_data[0] = pkt_index;
  usbeth_send(test_data,66);
  printf("\r\nusbeth_send: %d\r\n", pkt_index);
#elif TEST == 2
  pkt_index++;
  usbeth_send(test_data,64);
  printf("\r\nusbeth_send: %d\r\n", pkt_index);
#elif TEST == 3
  usbeth_l2_send(test_data, sizeof(test_data) - 4);
#endif

  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
shell_pure_init(void)
{
  shell_register_command(&pure_command);
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
    PRINTF("\n");
#if SERVER_REPLY
    PRINTF("DATA sending reply\n");
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    uip_udp_packet_send(server_conn, "Reply", sizeof("Reply"));
    uip_create_unspecified(&server_conn->ripaddr);
#endif
  }
}
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
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
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

  PRINTF("UDP server started. nbr:%d routes:%d\n",
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
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
