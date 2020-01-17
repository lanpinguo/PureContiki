/*
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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ip/uip-debug.h"


#ifdef WITH_COMPOWER
#include "powertrace.h"
#endif
#include <stdio.h>
#include <string.h>

/**************************************************************************/
/* from er-rest-example/er-rexample-client.c */
#include <stdlib.h>
#include "er-coap-engine.h"
#include "er-coap.h"
#include "er-coap-observe-client.h"
#include "coap-client.h"

#define MAX_PAYLOAD_LEN 	64


/* FIXME: This server address is hard-coded for Cooja and link-local for unconnected border router. */
/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xfe80, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202)      / * cooja2 * / */
/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1) */

/* #define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0x2000, 0, 0, 0, 0, 0, 0, 0x0001) */
#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT + 1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)

#define GET_INTERVAL 10
/* #define OBSERVE_INTERVAL 10 */

/**************************************************************************/

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define UDP_EXAMPLE_ID  190

/* #define DEBUG DEBUG_PRINT */
/* #include "net/ip/uip-debug.h" */

#ifndef PERIOD
/* #define PERIOD 60 */
#define PERIOD 10
#endif

#define START_INTERVAL    (15 * CLOCK_SECOND)
#define SEND_INTERVAL   (PERIOD * CLOCK_SECOND)
#define SEND_TIME   (random_rand() % (SEND_INTERVAL))

/*---------------------------------------------------------------------------*/
extern process_event_t dbg_event;
extern process_event_t nbr_chg_event;


/* static struct uip_udp_conn *client_conn; */
static uip_ipaddr_t server_ipaddr[MAX_SERVER_NUM] = {
	[0].u8 = {0xfd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,}
};

/**************************************************************************/

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 2
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_urls[NUMBER_OF_URLS] =
{ ".well-known/core", "/nbr" };




/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(void *response)
{
	const uint8_t *chunk;

	int len = coap_get_payload(response, &chunk);

	/*  printf("|%.*s", len, (char *)chunk); */
	printf("RX(%d):%s\r\n", len, (char *)chunk);
}



/**************************************************************************/


static void
generate_nbr_notify_payload(  uip_ipaddr_t *ipaddr, int opt, char *msg)
{
	int len;
	
	len = snprintf((char *)(msg), MAX_PAYLOAD_LEN, "&ip=");
	len += uip_ipaddr2str(ipaddr,msg,MAX_PAYLOAD_LEN - len);
	snprintf((char *)(msg + len), MAX_PAYLOAD_LEN - len, "&opt=%d", opt);
}



/*---------------------------------------------------------------------------*/
PROCESS(coap_client_process, "CoAP client process");
/*AUTOSTART_PROCESSES(&coap_client_process);*/
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    printf("DATA recv '%s'\r\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\r\n");
      /* hack to make address "final" */
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

}

/*---------------------------------------------------------------------------*/
/* The choice of server address determines its 6LoWPAN header compression.
 * (Our address will be compressed Mode 3 since it is derived from our
 * link-local address)
 * Obviously the choice made here must also be selected in udp-server.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the
 * 6LowPAN protocol preferences,
 * e.g. set Context 0 to fd00::. At present Wireshark copies Context/128 and
 * then overwrites it.
 * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit
 * compressed address of fd00::1111:22ff:fe33:xxxx)
 *
 * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
void
set_remote_server_address(uint32_t server_id, uip_ipaddr_t *ipaddr)
{
	uip_lladdr_t lladdr_aligned;
	int sum = 0,i;

	
	if(server_id >= MAX_SERVER_NUM){
		PRINTF("server id is invalid\r\n"); 
		return;
	}

	if(ipaddr == NULL){
		PRINTF("ipaddr is NULL\r\n"); 
		return;
	}

	for(i = 0 ; i < sizeof(uip_ipaddr_t); i++){
		sum += ipaddr->u8[i];
	}

	if(sum == 0){
		PRINTF("ipaddr is valid\r\n"); 
		return;
	}
	
	/*uip_ds6_addr_add(ipaddr, 0, ADDR_MANUAL);*/

	memcpy(&server_ipaddr[server_id], ipaddr, sizeof(uip_ipaddr_t));

	memcpy(&lladdr_aligned,&(ipaddr->u8[8]),UIP_LLADDR_LEN);
	lladdr_aligned.addr[0] ^= 0x02;
	uip_ds6_nbr_add(ipaddr, &lladdr_aligned,
		0, NBR_STALE, NBR_TABLE_REASON_UNDEFINED, NULL);


	PRINTF("CLIENT: SERVER[%d] IPv6 addresses: \r\n",(int)server_id); 
	PRINT6ADDR(&server_ipaddr[server_id]);
}

uip_ipaddr_t * get_remote_server_address(uint32_t server_id)
{
	if(server_id >= MAX_SERVER_NUM){
		PRINTF("server id is invalid\r\n"); 
		return NULL;
	}
	return(&server_ipaddr[server_id]);
}

/*---------------------------------------------------------------------------*/
#if 0
static void
generate_random_payload(int type, char *msg)
{
  if(type == 2) {   /* /dcdc/vdc */
    snprintf((char *)msg, 64, "&VG=%d&SL=%d&PMX=%d", (random_rand() % 25) + 1, (random_rand() % 10) + 1, (random_rand() % 100) + 1);
  } else if(type == 3) {  /* /dcdc/hwcfg */
    snprintf((char *)msg, 64, "&VMX=%d&IMX=%d", (random_rand() % 25) + 1, (random_rand() % 6) + 1);
  }
}
#endif




/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_client_process, ev, data)
{
	static COAP_CLIENT_ARG_t* p_coap_args = NULL;


	
	PROCESS_BEGIN();

	PROCESS_PAUSE();

	set_global_address();

	PRINTF("CoAP client process started\r\n");

	print_local_addresses();

	/**************************************************************************/
	static coap_packet_t request[1];      /* This way the packet can be treated as pointer as usual. */

	/* receives all CoAP messages */
	coap_init_engine();


	/* Indicate that The Coap Client Init has Done */
	//leds_on(1);


	while(1) {
		PROCESS_YIELD();


		if(ev == tcpip_event) {
			printf("TCPIP_HANDLER\r\n");
			tcpip_handler();
		}

		if(ev == nbr_chg_event) {
			static char msg[64] = "";
			static uip_ds6_nbr_t *nbr = NULL;
			static uip_ipaddr_t *ip_addr = NULL;

			coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
			coap_set_header_uri_path(request, service_urls[1]);

			printf("nbr_chg_event occur \r\n");
			if(data == NULL) {
				continue;
			}
			
			ip_addr = ( uip_ipaddr_t *)data;
			nbr = uip_ds6_nbr_lookup(ip_addr);



			if(nbr){
				generate_nbr_notify_payload(ip_addr,1,msg);
				//printf("nbr_add: \r\n");
				//PRINT6ADDR(ip_addr);
			}
			else{
				//printf("nbr_rm: \r\n");
				//PRINT6ADDR(ip_addr);
				generate_nbr_notify_payload(ip_addr,0,msg);
			}
			
			PRINTF("\r\nPOST: %s PAYLOAD: %s\r\n", service_urls[1], msg);
			coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);
			COAP_BLOCKING_REQUEST(&server_ipaddr[0], REMOTE_PORT, request,
					                  client_chunk_handler);
			
			continue;
		}

		if(ev == dbg_event) {

			if(data == NULL) {
				continue;
			}
			p_coap_args = (COAP_CLIENT_ARG_t*)data;
			printf("modid:%d,coap_conf:%d,coap_param:%d,server_id:%d\r\n"
					,p_coap_args->mod_id
					,p_coap_args->coap_conf
					,p_coap_args->coap_param
					,p_coap_args->server_id);
			PRINT6ADDR(&server_ipaddr[p_coap_args->server_id]);

			if(p_coap_args->mod_id == COAP_CLIENT_OWN){
				coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
				coap_set_header_uri_path(request, service_urls[0]);
				PRINTF("\r\nGET: %s\r\n", service_urls[0]);
				COAP_BLOCKING_REQUEST(&server_ipaddr[p_coap_args->server_id], REMOTE_PORT, request,
					                  client_chunk_handler);
			}
		}

  } /* END_WHILE(1) */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
