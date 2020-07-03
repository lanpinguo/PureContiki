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
/*#include "ping6.h"*/


#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

process_event_t dbg_event;
static struct etimer et;
static uint16_t sgp30_measure_value[2];
extern const struct sensors_sensor sgp30_sensor;


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
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
void shell_pure_init(void)
{
  shell_register_command(&pure_command);
  shell_register_command(&list_neighbor_command);
  shell_register_command(&dbg_sw_command);
}

void restore_config_from_file(){
	int rfd,rc;
	uip_ipaddr_t buf[10];


	rfd = cfs_open("config", CFS_READ);
	if(rfd < 0) {
		printf("\r\nopen failed fd=[%d] \r\n",rfd);	
		return;
	}

	/* Read buffer. */
	rc = cfs_read(rfd, buf, sizeof(uip_ipaddr_t) * 10);
	if(rc < 0) {
		printf("\r\nread failed fd=[%d] \r\n",rfd);	
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
	uint8_t buf[2] = {0x20,0x03};

	
	i2c_init(GPIO_D_NUM,1,GPIO_D_NUM,0,I2C_SCL_NORMAL_BUS_SPEED);
	printf("\r\ni2c_init done %x \r\n",(int)(2^16));

	/* Reset HTU-21 */
	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset chip error (%x)\r\n",rc);
		return -1;
	}


	/* Init_air_quality SGP-30 */
	rc = i2c_burst_send(0x58,buf,2);
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
		printf("Read temp error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[0],buf[1],buf[2],acc);
	}

	tmp = (buf[0]<<8) | (buf[1] & 0xFC);
	Temp = -46.85 + 175.72*(float)tmp/MAX_RANGE;

	goto no_error;



error_process:
	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset htu-21 chip error (%x)\r\n",rc);
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
		printf("receive humidity measure error(%x)\r\n",rc);
		goto error_process;
	}
	acc = CRC8(0,buf[0]);
	acc = CRC8(acc,buf[1]);

	if(acc != buf[2]){
		printf("Read humidity error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[0],buf[1],buf[2],acc);
	}

	tmp = (buf[0]<<8) | (buf[1] & 0xFC);
	RH = -6 + 125.0*((float)tmp/MAX_RANGE);


	goto no_error;



error_process:
	rc = i2c_single_send(0x40,0xfe);
	if(rc){
		printf("Reset htu-21 chip error (%x)\r\n",rc);
	}
	return 0;
no_error:
	return RH;
}


/*---------------------------------------------------------------------------*/
uint16_t sensors_get_co2(void)
{
	return sgp30_sensor.value(0);
}

/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
uint16_t sensors_get_tvoc(void)
{
#if 0
	uint8_t rc;
	uint8_t buf[8];
	uint16_t tmp;
	uint8_t acc;
  


	buf[0] = 0x20;
	buf[1] = 0x08;
	rc = i2c_burst_send(0x58,buf,2);
	if(rc){
		printf("Trigger tvoc measure error(%x)\r\n",rc);
		goto error_process;
	}

	rc = i2c_burst_receive(0x58,buf,6);
	if(rc){
		printf("receive tvoc measure error(%x)\r\n",rc);
		goto error_process;
	}
	acc = CRC8(0xff,buf[3]);
	acc = CRC8(acc,buf[4]);

	if(acc != buf[5]){
		printf("Read tvoc error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[3],buf[4],buf[5],acc);
	}

	tmp = (buf[3]<<8) | (buf[4]);

	goto no_error;



error_process:
	buf[0] = 0x20;
	buf[1] = 0x03;
	rc = i2c_burst_send(0x58,buf,2);
	if(rc){
		printf("init tvoc sensor error (%x)\r\n",rc);
	}
	return 0;
no_error:

	return tmp;
#endif	

	return 0;
}

PROCESS(sgp30_driver_process, "SGP30 Sensor Driver");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sgp30_driver_process, ev, data)
{
	static uint8_t buf[8];
	uint8_t rc;
	uint8_t acc;

	
	PROCESS_BEGIN();

	buf[0] = 0x20;
	buf[1] = 0x03;
	/* Init_air_quality SGP-30 */
	rc = i2c_burst_send(0x58,buf,2);
	if(rc){
		printf("Reset chip error (%x)\r\n",rc);
		return -1;
	}
	
	etimer_set(&et, CLOCK_SECOND * 2 );
	PROCESS_YIELD();

	while(1) 
	{

		buf[0] = 0x20;
		buf[1] = 0x08;
		rc = i2c_burst_send(0x58,buf,2);
		if(rc){
			printf("Trigger co2 measure error(%x)\r\n",rc);
			continue;
		}
		
		etimer_set(&et, CLOCK_SECOND / 5);
		PROCESS_YIELD();
		if(etimer_expired(&et)) {
			rc = i2c_burst_receive(0x58,buf,6);
			if(rc){
				printf("receive co2 measure error(%x)\r\n",rc);
				continue;
			}
			acc = CRC8(0xff,buf[0]);
			acc = CRC8(acc,buf[1]);

			if(acc != buf[2]){
				printf("Read co2 error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[0],buf[1],buf[2],acc);
			}
			sgp30_measure_value[0] = (buf[0]<<8) | (buf[1]);
			/*printf("Read co2 : %d \r\n",sgp30_measure_value[0]);*/

			acc = CRC8(0xff,buf[3]);
			acc = CRC8(acc,buf[4]);

			if(acc != buf[5]){
				printf("Read tvoc error: %02x%02x ,crc %02x, acc = %02x\r\n",buf[3],buf[4],buf[5],acc);
			}

			sgp30_measure_value[1] = (buf[3]<<8) | (buf[4]);
			/*printf("Read tvoc : %d \r\n",sgp30_measure_value[1]);*/
			

		}
		
		etimer_set(&et, CLOCK_SECOND * 5 );
		PROCESS_YIELD();
		
	}

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
static int
value(int type)
{
	if((uint32_t)type > 1){
		return 0;
	}
	return sgp30_measure_value[type];
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int value)
{
	switch(type) {
		case SENSORS_HW_INIT:

		break;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(sgp30_sensor, "SGP30", value, configure, status);



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
	&sgp30_driver_process,
	&coap_server_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
