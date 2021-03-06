#ifndef _OTA_TYPES_H_
#define _OTA_TYPES_H_

#include "net/ip/uip.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */



	// OTA Header constants
#define OTA_HDR_MAGIC_NUMBER                0x0BEEF11E
#define OTA_HDR_BLOCK_SIZE                  128
#define OTA_HDR_STACK_VERSION               2
#define OTA_HDR_HEADER_VERSION              0x0100
#define OTA_HDR_FIELD_CTRL                  0
	
#define OTA_HEADER_LEN_MIN                  56
#define OTA_HEADER_LEN_MAX                  69
#define OTA_HEADER_LEN_MIN_ECDSA            166
#define OTA_HEADER_STR_LEN                  32
	
#define OTA_HEADER_IMG_VALID              	0x5A5A
	
#define OTA_FC_SCV_PRESENT                  (0x1 << 0)
#define OTA_FC_DSF_PRESENT                  (0x1 << 1)
#define OTA_FC_HWV_PRESENT                  (0x1 << 2)

#define OTA_TRY_TIMES_MAX					5

	
#define HAL_IBM_LEDGER_PAGE        			254

#define NVIC_VECTOR_SIZE					(163*4)

#define IMG_MAX_NUMBER						3
	
#define IMG_HEADER_SIZE						(4 * 1024)
#define IMG_DATA_MAX_SIZE					(512 * 1024)

#define IMG_DEFAULLT_FACTORY_START 			0
#define IMG_DEFAULLT_FACTORY_END 			(4 * 1024)

#define IMG_BASE(i)							(i * (IMG_HEADER_SIZE  + IMG_DATA_MAX_SIZE) + IMG_DEFAULLT_FACTORY_END)
#define IMG_HEADER_START(i) 				IMG_BASE(i)
#define IMG_STATUS_OFFSET(i) 				(IMG_BASE(i)  + 64)
#define IMG_FLAG_DOMAIN_OFFSET(i) 			(IMG_BASE(i)  + 2 * 1024)
#define IMG_DATA_START(i)  					(IMG_BASE(i) + IMG_HEADER_SIZE)
#define IMG_DATA_CAPACITY(i) 				IMG_DATA_MAX_SIZE


#define FALSH_RESERVED_PARTION_START		(3 *(IMG_HEADER_SIZE + IMG_DATA_MAX_SIZE))
#define FALSH_RESERVED_PARTION_END			(2 * 1024 * 1024)

#if FALSH_RESERVED_PARTION_START > FALSH_RESERVED_PARTION_END
#error Flash reserved sector range is invalid 
#endif

#define FALSH_RPL_PARTION_START				FALSH_RESERVED_PARTION_END
#define FALSH_RPL_PARTION_SIZE				(2 * 1024 * 1024)


#define FLASH_FILE_SYSTEM_PARTION_OFFSET	(FALSH_RPL_PARTION_START + FALSH_RPL_PARTION_SIZE)
#define FLASH_FILE_SYSTEM_PARTION_SIZE		(12 * 1024 * 1024) /** 12MB **/

 
typedef struct
{
	uint16_t manufacturer;
	uint16_t type;
	uint32_t version;
} zclOTA_FileID_t;

typedef struct
{
	uint16_t tag;
	uint32_t length;
} OTA_SubElementHdr_t;

typedef struct
{
	uint32_t magicNumber;
	uint32_t imageSize;
	uint16_t imageValid;
	uint16_t headerLength;
	uint16_t headerVersion;
	uint16_t fieldControl;
	zclOTA_FileID_t fileId;
}__attribute__ ((packed)) OTA_ImageHeader_t;

typedef struct {
	uint32_t stackPtr;
	uint32_t nvicReset;
} ibm_ledger_t;
//static_assert((sizeof(ibm_ledger_t) == 16), "Need to PACK the ibm_ledger_t");

typedef enum {
	OTA_FRAME_TYPE_NONE 			= 0,
	OTA_FRAME_TYPE_UPGRADE_REQUEST 	= 1,
	OTA_FRAME_TYPE_DATA_REQUEST		= 2,
	OTA_FRAME_TYPE_FINISH			= 3,
	OTA_FRAME_TYPE_DATA				= 4,
	OTA_FRAME_TYPE_REBOOT			= 5,

}OTA_FrameType_e;

typedef enum {
	OTA_UPGRADE_OPTION_NONE         = 0,
	OTA_UPGRADE_OPTION_FORCE        = 1,
	OTA_UPGRADE_OPTION_RESTART      = 2,
	OTA_UPGRADE_OPTION_CONTINUE     = 3,
	OTA_UPGRADE_OPTION_CHECK        = 4,

}OTA_UPGRADE_OPTION_e;

typedef enum {
	OTA_STATE_NONE = 0,
	OTA_STATE_START ,
	OTA_STATE_RUNNING,
	OTA_STATE_FINISH,

}OTA_State_e;


typedef enum {
	OTA_EXT_IMG_STATUS_FRESH = 0xFFFFFFFF ,
	OTA_EXT_IMG_STATUS_STALE = 0x55AA55AA,
}OTA_ExtImageStatus_e;


typedef struct
{
	uint32_t 	magicNumber;
	uint32_t 	deviceType;
	uint32_t 	version;
	uint8_t 	primary;
	uint16_t 	seqno;
	uint32_t 	fileLen;		/* image file total length in bytes */
	uint16_t 	blockSize;		/* max size in a block */
	uint32_t 	checkCode;
	OTA_UPGRADE_OPTION_e option;
	OTA_State_e	state;
}__attribute__ ((packed)) OTA_FlashImageHeader_t;

typedef struct
{
	OTA_ExtImageStatus_e	status;
}__attribute__ ((packed)) OTA_FlashImageStatus_t;


typedef struct
{
	uint32_t 	deviceType;
	uint32_t 	version;
	uint8_t 	primary;
	uint16_t 	seqno;
	uint32_t 	fileLen;		/* image file total length in bytes */
	uint16_t 	blockSize;		/* max size in a block */
	uint32_t 	totalLen;		/* total length in bytes */
	uint32_t 	checkCode;
	uint32_t 	try_times_max;	/** Try times max when timeout**/
	OTA_UPGRADE_OPTION_e option;
	OTA_State_e	state;
	uip_ipaddr_t ripaddr;   	/**< The IP address of the remote peer. */
	uint16_t rport;        		/**< The remote port number in network byte order. */

}__attribute__ ((packed)) OTA_Info_t;


typedef struct
{
	uint8_t 	type;
	uint32_t 	magicNumber;
	uint32_t 	deviceType;
	uint32_t 	domain;
	uint32_t 	reboot;

}__attribute__ ((packed)) OTA_RebootRequestFrame_t;


typedef struct
{
	uint8_t 	type;
	uint32_t 	deviceType;
	uint32_t 	version;
	uint8_t 	primary;
	uint16_t 	blockSize;		/* max size in a block */
	uint32_t 	fileLen;		/* image file total length in bytes */
	OTA_UPGRADE_OPTION_e option;

}__attribute__ ((packed)) OTA_UpgradeRequestFrameHeader_t;

typedef struct
{
	uint8_t 	type;
	uint32_t 	deviceType;
	uint32_t 	version;
	uint16_t 	seqno;

}__attribute__ ((packed)) OTA_DataRequestFrame_t;

typedef struct
{
	uint8_t 	type;
	uint32_t 	deviceType;
	uint32_t 	version;
	uint16_t 	seqno;
	uint32_t 	checkCode;
	uint8_t 	state;

}__attribute__ ((packed)) OTA_FinishFrameHeader_t;

typedef struct
{
	uint8_t 	type;
	uint32_t 	deviceType;
	uint32_t 	version;
	uint16_t 	seqno;
	uint8_t 	dataLength;
	uint8_t 	data[0];
}__attribute__ ((packed)) OTA_DataFrameHeader_t;

#endif /* _OTA_TYPES_H_*/


