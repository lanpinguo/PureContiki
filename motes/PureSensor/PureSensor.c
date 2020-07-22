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

#include "util.h"
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


PROCESS_NAME(shell_debug_process);
PROCESS_NAME(ota_upgrade_process);

#if MAC_USING_TSCH
PROCESS_NAME(node_process);
#endif
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/


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
AUTOSTART_PROCESSES(
	&shell_debug_process,
	&ota_upgrade_process,

	&pure_x_shell_process,
#if MAC_USING_TSCH
	&node_process,
#endif
	&sgp30_driver_process,
	&coap_server_process);



/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
