/*  
 * @file     nblockProtcl.h	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018.03.27 
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _NBLOCKPROTCL_H_
#define _NBLOCKPROTCL_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdint.h>


#define NBLOCK_PROTCL_HEADER_SIZE   (18)
#define NBLOCK_PROTCL_PAYLOAD_SIZE  (110)
#define NBLOCK_RESPDATA_SIZE        NBLOCK_PROTCL_PAYLOAD_SIZE
#define SERIAL_PARSE_BUFFER_SIZE    (256)

#define URI_IP                      "101.132.153.33"
#define URI_PORT                    "5566"
#define URI_IP_PORT					        "coap://"URI_IP":"URI_PORT


/* NBIoT 私有协议CMD */
typedef enum
{
   /* down */
   NBLOCK_DOWNCMD_OPENLOCK      = 0x05,        /* open door */
   NBLOCK_DOWNCMD_ADDATTR       = 0x06,        /* add attribute */
   NBLOCK_DOWNCMD_DELATTR       = 0x07,        /* delete attribute */
   NBLOCK_DOWNCMD_MODATTR       = 0x08,        /* modify attribute  */
   NBLOCK_DOWNCMD_QRYATTR       = 0x09,        /* query attribute */
   NBLOCK_DOWNCMD_OTASTART      = 0x0B,        /* OTA Start */ 
   NBLOCK_DOWNCMD_NONE          = 0x1f,        /* no operate */
   /* up */
   NBLOCK_PING                  = 0x04,        /* ping */
   NBLOCK_UPCMD_PERMIT          = 0x02,        /* Request to join */
   NBLOCK_UPCMD_NETSERVICE      = 0x03,        /* net service */
   NBLOCK_UPCMD_RPTATTR         = 0x0a,        /* report attributes */
   NBLOCK_UPCMD_OTADATA         = 0x0c,        /* OTA Data */
}NbiotLockCmd_t;


/* Serialize Struct */
typedef struct
{
    uint16_t port;
    char     ip[64];
    uint8_t  buffer[SERIAL_PARSE_BUFFER_SIZE];
    uint16_t bufferLen;
}NblockSerial_t;

/* Parse Struct */
typedef struct
{
    uint8_t  buffer[SERIAL_PARSE_BUFFER_SIZE];
    uint16_t bufferLen;
}NblockParse_t;

/* Frame Struct */
typedef struct
{
    uint8_t cmd;
    uint8_t dataLen;
    uint16_t seq;
    uint8_t data[NBLOCK_PROTCL_PAYLOAD_SIZE];
}NblockFrame_t;


int nblockProtclSerial(NblockFrame_t *frame, NblockSerial_t *serial);

int nblockProtclParse(NblockFrame_t *frame, NblockParse_t *parse);

int nblockProtclInit(const uint8_t mac[], uint16_t macSize);


#ifdef __cplusplus 
} 
#endif

#endif /* nblockProtcl.h */
