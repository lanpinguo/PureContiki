/*---------------------------------------------------------------------------*/
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
/*---------------------------------------------------------------------------*/
#include "ota_types.h"

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_NONE
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

	printf("device type: %lu, version: %lu, fileLen : %lu\r\n",
			frame->deviceType,
			frame->version,
			frame->fileLen);
	buffer_dump((uint8_t *)data,uip_datalen());


	ota_info_current.fileLen = frame->fileLen;
	ota_info_current.blockSize = frame->blockSize;
	ota_info_current.state = OTA_STATE_RUNNING;
	
	req.type = OTA_FRAME_TYPE_DATA_REQUEST;
	req.deviceType = ota_info_current.deviceType;
	req.version = ota_info_current.version;
	req.seqno = ota_info_current.seqno = 0;
	
    uip_ipaddr_copy(&tx_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
	tx_conn->rport = UIP_UDP_BUF->srcport;
	uip_udp_packet_send(tx_conn, &req, sizeof(req));

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
				xmem_pwrite(frame->data,
							frame->dataLength,
							IMG_1_DATA_START + frame->seqno * ota_info_current.blockSize);
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
					
					xmem_pread(buf, len, IMG_1_DATA_START + i);
					
					checkCode = crc32_data(buf, len, checkCode);

					i += len;
				}

				if(checkCode == frame->checkCode){
					OTA_FlashImageHeader_t *imgHeader = (OTA_FlashImageHeader_t*)buf;

					imgHeader->deviceType 	= ota_info_current.deviceType;
					imgHeader->version 		= ota_info_current.version;
					imgHeader->fileLen		= ota_info_current.fileLen;
					imgHeader->checkCode	= ota_info_current.checkCode;
					imgHeader->blockSize	= ota_info_current.blockSize;
					xmem_pwrite(buf,
								sizeof(OTA_FlashImageHeader_t),
								IMG_1_HEADER_START);


				}
				printf("total lentgh : %lu, Local Check Code: 0x%08lx\r\n",ota_info_current.totalLen, checkCode);
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

		/*buffer_dump((uint8_t *)appdata,uip_datalen());*/
		OTA_StateMachineUpdate(appdata,0);
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

