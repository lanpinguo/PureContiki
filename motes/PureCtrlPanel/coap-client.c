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

#if PLATFORM_HAS_LEDS
#include "dev/leds.h"
#endif

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

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

/* static struct uip_udp_conn *client_conn; */
static uip_ipaddr_t server_ipaddr[MAX_SERVER_NUM];
static int count_notify = 0;

/**************************************************************************/
static struct etimer et;

/* Example URIs that can be queried. */
#define NUMBER_OF_URLS 6
/* leading and ending slashes only for demo purposes, get cropped automatically when setting the Uri-Path */
char *service_urls[NUMBER_OF_URLS] =
{ ".well-known/core", "/dcdc/status", "/dcdc/vdc", "/dcdc/hwcfg","relay-sw","hcho" };


static int btn_pressed = 0;
static int btn_state[6];

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(void *response)
{
	const uint8_t *chunk;

	int len = coap_get_payload(response, &chunk);

#if PLATFORM_HAS_LEDS
	if(btn_state[btn_pressed]){
		/* set led when receiving a packet */
		leds_on((1<<(btn_pressed + 1)));
	}
	else{
		leds_off((1<<(btn_pressed + 1)));
	}
#endif

	/*  printf("|%.*s", len, (char *)chunk); */
	printf("RX(%d):%s\r\n", len, (char *)chunk);
}

/* This function is will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_relay_state_chunk_handler(void *response)
{
	const uint8_t *chunk;
	const char *state = NULL;
	const char *mask = NULL;
	uint32_t sw_mask = 0;
	uint32_t sw_state = 0;

	int len = coap_get_payload(response, &chunk);

#if PLATFORM_HAS_LEDS
	if(btn_state[btn_pressed]){
		/* set led when receiving a packet */
		leds_on((1<<(btn_pressed + 1)));
	}
	else{
		leds_off((1<<(btn_pressed + 1)));
	}
#endif

	len = REST.get_post_variable((char *)chunk, "state", &state);
	if(len > 0) {
		sw_state = strtoul(state, NULL, 16);
	} 

	len = REST.get_post_variable((char *)chunk, "mask", &mask);
	if(len > 0) {
		sw_mask = strtoul(mask, NULL, 16);
	}
	printf("state=%lx,mask=%lx\r\n", sw_state,sw_mask);

	/*  printf("|%.*s", len, (char *)chunk); */
	printf("RX(%d):%s\r\n", len, (char *)chunk);
}

/**************************************************************************/

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

static void
generate_relay_sw_config_payload(int mask, int state, char *msg)
{
   /* relay-sw */
	snprintf((char *)msg, 64, "&state=%d&mask=%d", state,mask);
}

/*----------------------------------------------------------------------------*/
/*
 * Handle the response to the observe request and the following notifications
 */
static void
notification_callback(coap_observee_t *obs, void *notification, coap_notification_flag_t flag)
{
  int len = 0;
  const uint8_t *payload = NULL;

/*  printf("Notification handler\r\n"); */
/*  printf("Observee URI: %s\r\n", obs->url); */
  if(notification) {
    len = coap_get_payload(notification, &payload);
  }
  switch(flag) {
  case NOTIFICATION_OK:
    count_notify++;
    printf("NOTIFICATION OK: %d %*s\r\n", count_notify, len, (char *)payload);
    break;
  case OBSERVE_OK: /* server accepeted observation request */
    printf("OBSERVE_OK: %*s\r\n", len, (char *)payload);
    break;
  case OBSERVE_NOT_SUPPORTED:
    printf("OBSERVE_NOT_SUPPORTED: %*s\r\n", len, (char *)payload);
    obs = NULL;
    break;
  case ERROR_RESPONSE_CODE:
    printf("ERROR_RESPONSE_CODE: %*s\r\n", len, (char *)payload);
    obs = NULL;
    break;
  case NO_REPLY_FROM_SERVER:
    printf("NO_REPLY_FROM_SERVER: "
           "removing observe registration with token %x%x\r\n",
           obs->token[0], obs->token[1]);
    obs = NULL;
    break;
  }
}


/*---------------------------------------------------------------------------*/
extern process_event_t dbg_event;

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_client_process, ev, data)
{
/*
   #if WITH_COMPOWER
   static int print = 0;
   #endif
 */
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

	etimer_set(&et, GET_INTERVAL * CLOCK_SECOND);

	/* Indicate that The Coap Client Init has Done */
	leds_on(1);


	while(1) {
		PROCESS_YIELD();

		if(ev == tcpip_event) {
			printf("TCPIP_HANDLER\r\n");
			tcpip_handler();
		}

#if PLATFORM_HAS_BUTTON
		if (ev == sensors_event ) {
			static char msg[64] = "";
			static int mask,state;

			coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
			coap_set_header_uri_path(request, service_urls[4]);
			
			if(data == &button_cancel_sensor){
				btn_pressed = 0;
				mask = (1<< 0) ;
			}else if(data == &button_select_sensor){
				btn_pressed = 1;
				mask = (1<<1) | (1<<2) | (1<< 3) | (1<<4) ;
			}else if(data == &button_left_sensor){
				btn_pressed = 2;
				mask = (1<< 5) | (1<<6) | (1<<7);
			}else if(data == &button_right_sensor){
				btn_pressed = 3;
			}else if(data == &button_up_sensor){
				btn_pressed = 4;
			}else if(data == &button_down_sensor){
				btn_pressed = 5;
			}
			else{
				PRINTF("\r\nInvalid Button Pressed \r\n");
				continue;
			}

			if(mask == 0){
				continue;
			}
			
			PRINTF("\r\nButton[%d] Pressed \r\n",btn_pressed);
			btn_state[btn_pressed] = ~btn_state[btn_pressed];
			state = btn_state[btn_pressed] > 0 ? ~mask : mask;
			generate_relay_sw_config_payload(mask,state,msg);
			coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);
			PRINTF("\r\nPUT: %s PAYLOAD: %s\r\n", service_urls[4], msg);
			COAP_BLOCKING_REQUEST(&server_ipaddr[0], REMOTE_PORT, request,
								  client_chunk_handler);
		}
#endif

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
			else if (p_coap_args->mod_id == COAP_CLIENT_SW_ST){
				coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
				coap_set_header_uri_path(request, service_urls[4]);
				PRINTF("\r\nGET: %s\r\n", service_urls[4]);
				COAP_BLOCKING_REQUEST(&server_ipaddr[p_coap_args->server_id], REMOTE_PORT, request,
					                  client_relay_state_chunk_handler);
			}
			else if (p_coap_args->mod_id == COAP_CLIENT_HCHO){
				coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
				coap_set_header_uri_path(request, service_urls[5]);
				PRINTF("\r\nGET: %s\r\n", service_urls[5]);
				COAP_BLOCKING_REQUEST(&server_ipaddr[p_coap_args->server_id], REMOTE_PORT, request,
					                  client_chunk_handler);
			}
			else if (p_coap_args->mod_id == COAP_CLIENT_SW){
				char msg[64] = "";
				static int mask,state;
				coap_init_message(request, COAP_TYPE_CON, COAP_PUT, 0);
				coap_set_header_uri_path(request, service_urls[4]);
				state = (p_coap_args->coap_param > 0) ? (1<<p_coap_args->coap_conf):0;
				mask = (1<<p_coap_args->coap_conf);
				generate_relay_sw_config_payload(mask,state,msg);
				coap_set_payload(request, (uint8_t *)msg, sizeof(msg) - 1);
				PRINTF("\r\nPUT: %s PAYLOAD: %s\r\n", service_urls[4], msg);
				COAP_BLOCKING_REQUEST(&server_ipaddr[p_coap_args->server_id], REMOTE_PORT, request,
					                  client_chunk_handler);


			}
		}

  } /* END_WHILE(1) */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
