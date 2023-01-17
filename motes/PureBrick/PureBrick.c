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
#include "util.h"
#include "hcho-sensor.h"

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_NONE
#include "net/ip/uip-debug.h"
#include "relay_switch.h"
#include "dev/leds.h"


#define RELAY_SW_ACTIVE_TIME	(60 * 60 * 6) /* 6 hours */
#define RELAY_SW_DEACTIVE_TIME	(60 * 60 * 18) /* 18 hours */

process_event_t dbg_event;



/*
static struct etimer et;
static uint32_t sw_on_flag = 1;
*/

#if MAC_USING_TSCH
PROCESS_NAME(node_process);
#endif
PROCESS_NAME(shell_debug_process);
PROCESS_NAME(ota_upgrade_process);
PROCESS_NAME(telnetd_process);

#define PCF8547_ADDR_BASE       0x20
#define SWITCH_NUM              8


int32_t i2c_switch_set(uint32_t ch, uint32_t val)
{
	int32_t rc;
	uint8_t buf[2] = {0,0};

    if(val > 0){
        buf[0] = 0xFF;
    }
    else{
        buf[0] = 0xFC;
    }
    rc = i2c_single_send(PCF8547_ADDR_BASE + ch, buf[0]);
    if(rc){
        printf("PCF8574 write in channel %ld, error (%lx)\r\n", ch, rc);
    }
    else{
        printf("ch %ld state: 0x%02x\r\n", ch, buf[0]);
    }

    return rc;

}

int32_t i2c_switch_get(uint32_t ch, uint32_t* val)
{
	uint8_t buf[2] = {0,0};
	int32_t rc;

    rc = i2c_single_receive(PCF8547_ADDR_BASE + ch, buf);
    if(rc){
        printf("PCF8574 get in channel %ld, error (%lx)\r\n",ch, rc);
    }
    else{
        printf("ch %ld state: 0x%02x\r\n", ch, buf[0]);
    }

    if((buf[0] & 0x02) == 0x02){
        *val = 1;
    }
    else{
        *val = 0;
    }

    return rc;
}

int32_t
relay_switch_get_all(uint32_t *state, uint32_t *mask)
{
	int32_t rc;
	uint32_t val;
	int i;

	
	if(state == NULL || mask == NULL){
		return -1;
	}

	/*clean first*/
	*state = 0;
	*mask = 0;

	
	for(i = 0 ; i < SWITCH_NUM; i++){
        rc = i2c_switch_get(i, &val);
        if(rc == 0){
            if(val > 0){
                *state |= (1<<i);
            }
            *mask |= (1<<i);
        }
		/*printf("[%d] = %x, state = %x\r\n",i,val,*state);*/
	}


	return 0;	
}

void
relay_switch_set(uint8_t sw, uint8_t value)
{
	if(sw > (SWITCH_NUM - 1)){
		return ;
	}

    i2c_switch_set(sw, value);
}

/*---------------------------------------------------------------------------*/
int i2c_switch_init(void)
{
	uint8_t rc;
    uint32_t val;   
	
	i2c_init(GPIO_D_NUM,1,GPIO_D_NUM,0,I2C_SCL_NORMAL_BUS_SPEED);
    ioc_set_over(GPIO_D_NUM, 0, IOC_OVERRIDE_PUE);
    ioc_set_over(GPIO_D_NUM, 1, IOC_OVERRIDE_PUE);
    printf("\r\ni2c_init done\r\n");

	/* Scan PCF8574 */
    for(int i = 0 ; i < 8 ; i++){
        rc = i2c_switch_get(i, &val);
        if(rc){
            printf("PCF8574 scan in channel %d, error (%x)\r\n",i, rc);
        }
        else{
            printf("ch %d state: %ld\r\n", i,val);
        }
    }


	return 0;
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
			// relay_switch_set(i,(sw_state & sw_mask));
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
	shell_register_command(&coap_client_command);

#if COFFEE
	shell_coffee_init();
	shell_file_init();
#endif

	hcho_sensor_init(1);  

    i2c_switch_init();

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




/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(
	&telnetd_process,
	&shell_debug_process,
	&ota_upgrade_process,
	&pure_x_shell_process,
#if MAC_USING_TSCH
	&node_process,
#endif
	&coap_server_process, 
	&coap_client_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
