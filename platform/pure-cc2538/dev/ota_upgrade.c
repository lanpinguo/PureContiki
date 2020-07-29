/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "cfs/cfs.h"
#include "net/netstack.h"
#include "dev/watchdog.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "crc32.h"

#include "xmem.h"
/*---------------------------------------------------------------------------*/
#include "ota_types.h"

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_OTA
#include "net/ip/uip-debug.h"

extern int show_system_info(uint32_t mode);

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF       ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])

#define UDP_SERVER_PORT 		5678
#define UDP_EXAMPLE_ID  		190

static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *tx_conn;

#define FLASH_BUF_LEN	512
static uint8_t buf[FLASH_BUF_LEN]; /* flash shared buf */





static OTA_Info_t ota_info_current = {
	.deviceType = OTA_DEVICE_TYPE,
	.version 	= OTA_FIRMWARE_VERSION,
	.primary 	= 1,
	.seqno 		= 0,
	.totalLen 	= 0,
	.state 		= OTA_STATE_NONE,
};

static struct etimer ota_timeout;


void BUFFER_DUMP(uint8_t * buf, uint32_t len)
{
  uint16_t i;

  
  //printf("block  %lu:", len);
  for (i = 0; i < len ; i++) {
    uint8_t data = buf[i];
	if(i % 16 == 0){
      PRINTF("\r\n");
	}
    PRINTF("%02x ", data);
  }
  PRINTF("\r\n");
}


int32_t OTA_UpgradeStart(char * data)
{
	OTA_DataRequestFrame_t req;

	OTA_UpgradeRequestFrameHeader_t * frame = (OTA_UpgradeRequestFrameHeader_t *)data;

	PRINTF("device type: %lu, version: %lu, fileLen : %lu\r\n",
			frame->deviceType,
			frame->version,
			frame->fileLen);
	BUFFER_DUMP((uint8_t *)data,uip_datalen());

	if(ota_info_current.deviceType != frame->deviceType){
		PRINTF("device type mismatch %lu/%lu \r\n",frame->deviceType, ota_info_current.deviceType);
		return -1;
	}

	if(frame->primary >= IMG_MAX_NUMBER){
		return -1;
	}
	
	PRINTF("erase ext-img: %u \r\n",frame->primary);
	/* Erase external flash pages */
	xmem_erase(IMG_HEADER_SIZE + IMG_DATA_MAX_SIZE, IMG_BASE(frame->primary));

	ota_info_current.fileLen = frame->fileLen;
	ota_info_current.blockSize = frame->blockSize;
	ota_info_current.state = OTA_STATE_RUNNING;
	ota_info_current.primary = frame->primary;
	ota_info_current.try_times_max = OTA_TRY_TIMES_MAX;
	ota_info_current.totalLen = 0;
	ota_info_current.rport  = UIP_UDP_BUF->srcport;
    uip_ipaddr_copy(&ota_info_current.ripaddr, &UIP_IP_BUF->srcipaddr);

	
	req.type = OTA_FRAME_TYPE_DATA_REQUEST;
	req.deviceType = ota_info_current.deviceType;
	req.version = ota_info_current.version;
	req.seqno = ota_info_current.seqno = 0;
	
    uip_ipaddr_copy(&tx_conn->ripaddr, &ota_info_current.ripaddr);
	tx_conn->rport = ota_info_current.rport;
	uip_udp_packet_send(tx_conn, &req, sizeof(req));

	return 0;

}


int32_t OTA_StateMachineUpdate(char* data, uint32_t event)
{
	static uint32_t pkt_type;


	pkt_type = OTA_FRAME_TYPE_NONE;
	if(data != NULL){
		pkt_type = data[0];
		PRINTF("pkt type : %u\r\n",pkt_type);
	}

	if(ota_info_current.state == OTA_STATE_NONE){
		
		if(pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST){
			
			OTA_UpgradeStart(data);
		}
		else if(pkt_type == OTA_FRAME_TYPE_REBOOT){
			
			OTA_RebootRequestFrame_t * frame = (OTA_RebootRequestFrame_t *)data;
			if(frame->magicNumber == OTA_HDR_MAGIC_NUMBER){
				if(frame->domain != 0){
					if(frame->deviceType != ota_info_current.deviceType){
						return 0;
					}
				}
				if(frame->reboot == 0x55aa55aa){
					watchdog_reboot();
				}
		}
	}

	}
	else if (ota_info_current.state == OTA_STATE_RUNNING){
		
		if(pkt_type == OTA_FRAME_TYPE_DATA ||
			pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST ||
			pkt_type == OTA_FRAME_TYPE_FINISH ){
			
			static OTA_DataRequestFrame_t req;
			OTA_DataFrameHeader_t * frame = (OTA_DataFrameHeader_t *)data;
			static char flag;

			if (pkt_type == OTA_FRAME_TYPE_UPGRADE_REQUEST){
				OTA_UpgradeRequestFrameHeader_t * frame = (OTA_UpgradeRequestFrameHeader_t *)data;

				if(frame->option == OTA_UPGRADE_OPTION_RESTART){
					OTA_UpgradeStart(data);
				}

			}
			
			PRINTF("seqno %u, data len: %u\r\n",frame->seqno, frame->dataLength);
			//buffer_dump((uint8_t *)data,uip_datalen());
			/* Write data into flash */
			xmem_pwrite_raw(frame->data,
							frame->dataLength,
							IMG_DATA_START(ota_info_current.primary) + frame->seqno * ota_info_current.blockSize);
			/* Set flag indicating current block is programed into ext-flash */
			flag  = ~(1<<(frame->seqno % 8));
			xmem_pwrite_raw( &flag,
							 1,
							 IMG_FLAG_DOMAIN_OFFSET(ota_info_current.primary) + frame->seqno / 8 );
			
			ota_info_current.totalLen += frame->dataLength;

			if(ota_info_current.totalLen >= ota_info_current.fileLen){
				ota_info_current.state = OTA_STATE_FINISH;
			}
			/* Request next block */
			req.type = OTA_FRAME_TYPE_DATA_REQUEST;
			req.deviceType = ota_info_current.deviceType;
			req.version = ota_info_current.version;
			req.seqno = ++ota_info_current.seqno;

			if(	pkt_type == OTA_FRAME_TYPE_FINISH ){
				ota_info_current.state = OTA_STATE_FINISH;
			}
			
		    uip_ipaddr_copy(&tx_conn->ripaddr, &ota_info_current.ripaddr);
			tx_conn->rport = ota_info_current.rport;
			uip_udp_packet_send(tx_conn, &req, sizeof(req));

			etimer_set(&ota_timeout,  CLOCK_SECOND * 3);
		}
	}
	else if (ota_info_current.state == OTA_STATE_FINISH){
		
		if(pkt_type == OTA_FRAME_TYPE_FINISH){
			OTA_FinishFrameHeader_t * frame = (OTA_FinishFrameHeader_t *)data;
			uint32_t checkCode = 0xFFFFFFFF;
			static uint32_t i;
			

			etimer_stop(&ota_timeout);
			
			PRINTF("seqno %u, checkCode: 0x%08lx\r\n",frame->seqno, frame->checkCode);
			//buffer_dump((uint8_t *)data,uip_datalen());
			ota_info_current.checkCode = frame->checkCode;
			/* re-generate check code */
			for(i = 0; i < ota_info_current.fileLen; ){
				int len = FLASH_BUF_LEN;

				if(ota_info_current.fileLen - i < FLASH_BUF_LEN){
					len = ota_info_current.fileLen - i ;
				}
					
				if(len <= 0){
					break;
				}
				
				xmem_pread_raw(buf, len, IMG_DATA_START(ota_info_current.primary) + i);
				
				checkCode = crc32_data(buf, len, checkCode);

				i += len;
			}

			if(checkCode == frame->checkCode){
				OTA_FlashImageHeader_t *imgHeader = (OTA_FlashImageHeader_t*)buf;

				imgHeader->magicNumber	= OTA_HDR_MAGIC_NUMBER;
				imgHeader->deviceType 	= ota_info_current.deviceType;
				imgHeader->version 		= ota_info_current.version;
				imgHeader->fileLen		= ota_info_current.fileLen;
				imgHeader->checkCode	= ota_info_current.checkCode;
				imgHeader->blockSize	= ota_info_current.blockSize;

				
				xmem_pwrite_raw(buf,
							sizeof(OTA_FlashImageHeader_t),
							IMG_HEADER_START(ota_info_current.primary));

			}
			else{
				/** Notify server the image check failed **/
				OTA_FinishFrameHeader_t req;
				/* Request next block */
				req.type = OTA_FRAME_TYPE_FINISH;
				req.deviceType = ota_info_current.deviceType;
				req.version = ota_info_current.version;
				req.seqno = ota_info_current.seqno;
				req.state = 0xFF; /** check failed **/
				
			    uip_ipaddr_copy(&tx_conn->ripaddr, &ota_info_current.ripaddr);
				tx_conn->rport = ota_info_current.rport;
				uip_udp_packet_send(tx_conn, &req, sizeof(req));
			}
			PRINTF("total lentgh : %lu, Local Check Code: 0x%08lx\r\n",ota_info_current.totalLen, checkCode);
			ota_info_current.state = OTA_STATE_NONE;
		}

	}

	return 0;
}

static void try_to_request_data_again(void)
{

	static OTA_DataRequestFrame_t req;



	if (ota_info_current.state != OTA_STATE_RUNNING){
		return;
	}

	if(ota_info_current.try_times_max == 0){
		ota_info_current.state = OTA_STATE_NONE;
	}

	ota_info_current.try_times_max--;
	
	/* Request current block */
	req.type 		= OTA_FRAME_TYPE_DATA_REQUEST;
	req.deviceType 	= ota_info_current.deviceType;
	req.version 	= ota_info_current.version;
	req.seqno 		= ota_info_current.seqno;


    uip_ipaddr_copy(&tx_conn->ripaddr, &ota_info_current.ripaddr);
	tx_conn->rport = ota_info_current.rport;
	uip_udp_packet_send(tx_conn, &req, sizeof(req));

	printf("try to request data through tx-connection with remote address ");
	uip_ipaddr_print(&tx_conn->ripaddr);
	printf("local/remote port %u/%u\r\n",
			UIP_HTONS(tx_conn->lport),
			UIP_HTONS(tx_conn->rport));


	etimer_set(&ota_timeout,  CLOCK_SECOND * 3);

}


static void
ota_packet_handler(void)
{
	static char *appdata;

	PRINTF("ota_packet_handler enter \r\n");
	if(uip_newdata()) {

		appdata = (char *)uip_appdata;
		appdata[uip_datalen()] = 0;

		BUFFER_DUMP((uint8_t *)appdata,uip_datalen());
		
		OTA_StateMachineUpdate(appdata,0);
	}
	else{
		PRINTF("ota_packet_handler no new data :%u \r\n",uip_newdata());
	}
}



/*---------------------------------------------------------------------------*/
PROCESS(ota_upgrade_process, "ota-upgrade");

PROCESS_THREAD(ota_upgrade_process, ev, data)
{

	PROCESS_BEGIN();

	PROCESS_PAUSE();


	PRINTF("\r\nUDP server started. nbr:%d routes:%d \r\n",
			NBR_TABLE_CONF_MAX_NEIGHBORS,
			UIP_CONF_MAX_ROUTES);

	//print_local_addresses();


	server_conn = udp_new(NULL, 0, NULL);
	if(server_conn == NULL) {
		PRINTF("No UDP connection available, exiting the process!\r\n");
		PROCESS_EXIT();
	}
	
	udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

	PRINTF("Created a server connection with local/remote port %u/%u\r\n",
			UIP_HTONS(server_conn->lport),
			UIP_HTONS(server_conn->rport));


	tx_conn = udp_new(NULL, 0, NULL);
	if(tx_conn == NULL) {
		PRINTF("No UDP connection available, exiting the process!\r\n");
		PROCESS_EXIT();
	}


	while(1) {
		
		PROCESS_YIELD();
		
		if(ev == tcpip_event) {
			ota_packet_handler();
		}

		if(ev == PROCESS_EVENT_TIMER){
			if(etimer_expired(&ota_timeout)) {
				PRINTF("ota_timeout expired\r\n");
				try_to_request_data_again();
			}
		}

	}

	PROCESS_END();
}

