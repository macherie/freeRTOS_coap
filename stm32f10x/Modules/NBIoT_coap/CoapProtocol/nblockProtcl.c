/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018.03.27 
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "crc_cal.h"
#include "nblockProtcl.h"
#include "coap_client.h"


#define DEBUG 1

#define PROTCL_HEAD1        0x01
#define PROTCL_HEAD2        0xff
#define UUID_SIZE           8
#define TXRXBUFFER_SIZE     (NBLOCK_PROTCL_HEADER_SIZE + NBLOCK_PROTCL_PAYLOAD_SIZE)

#define DEVICE_TYPE         (0x1207)    /* device type */

#define URI_RSPDOWN_ADDATTR     URI_IP_PORT"/rsp_addattr"       
#define URI_RSPDOWN_DELATTR     URI_IP_PORT"/rsp_delattr"       
#define URI_RSPDOWN_MODATTR     URI_IP_PORT"/rsp_modattr"
#define URI_RSPDOWN_QRYATTR     URI_IP_PORT"/rsp_qryattr"
#define URI_RSPDOWN_OPENLOCK    URI_IP_PORT"/rsp_open"
#define URI_RSPDOWN_OTASTART    URI_IP_PORT"/rsp_otastart"
#define URI_UPCMD_PERMIT        URI_IP_PORT"/permit"
#define URI_UPCMD_PING          URI_IP_PORT"/ping"
#define URI_UPCMD_NETSERVICE    URI_IP_PORT"/netservice"
#define URI_UPCMD_RPTATTR       URI_IP_PORT"/rptattr"
#define URI_UPCMD_OTADATA       URI_IP_PORT"/otadata"


typedef struct{
    uint8_t     initd;
    uint8_t     uuid[UUID_SIZE];
    uint16_t    deviceType;
    
}NblockProtclCtx_t;

typedef struct
{
    uint8_t cmd;
    char    *uriPtr;
}Cmd2Uri_t;


static Cmd2Uri_t gCmdUri[]={
    /* up */
    {NBLOCK_PING,                   URI_UPCMD_PING},
    {NBLOCK_UPCMD_PERMIT,           URI_UPCMD_PERMIT},
    {NBLOCK_UPCMD_NETSERVICE,       URI_UPCMD_NETSERVICE},
    {NBLOCK_UPCMD_RPTATTR,          URI_UPCMD_RPTATTR},
    {NBLOCK_UPCMD_OTADATA,          URI_UPCMD_OTADATA},
    /* down */
    {NBLOCK_DOWNCMD_OPENLOCK,    URI_RSPDOWN_OPENLOCK},  
    {NBLOCK_DOWNCMD_ADDATTR,     URI_RSPDOWN_ADDATTR},
    {NBLOCK_DOWNCMD_DELATTR,     URI_RSPDOWN_DELATTR},
    {NBLOCK_DOWNCMD_MODATTR,     URI_RSPDOWN_MODATTR},  
    {NBLOCK_DOWNCMD_QRYATTR,     URI_RSPDOWN_QRYATTR},
    {NBLOCK_DOWNCMD_OTASTART,    URI_RSPDOWN_OTASTART}
};
static uint8_t gTxRxBuffer[TXRXBUFFER_SIZE];
static NblockProtclCtx_t gCtx;


/*  
 * @brief      cmd match URI   
 * @param[out] uriPtrPtr -- uri pointer pointer  
 * @param[in]  cmd -- up cmd 
 * @return     0 -- found; 1 -- not found  
 * @see         
 * @note        
 */
static int cmdMatchUri(uint8_t cmd, char **uriPtrPtr)
{
    uint16_t idx;

    for (idx = 0;idx < sizeof(gCmdUri)/sizeof(gCmdUri[0]);idx++)
    {
        if (cmd == gCmdUri[idx].cmd)
        {
            *uriPtrPtr = gCmdUri[idx].uriPtr;
            return 0;
        }
    }
    return 1;
}
/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int makePkt(uint8_t cmd, uint16_t seq, const uint8_t data[], uint8_t dataLen, uint8_t buffer[], uint16_t bufferSize)
{
    uint16_t idx = 0;
    
    if (NBLOCK_PROTCL_HEADER_SIZE > bufferSize)
    {
        LOGERROR("Buffer Size too Small.\n");
        return 0;
    }
    if (dataLen > NBLOCK_PROTCL_PAYLOAD_SIZE)
    {
        LOGERROR("Data too Big.\n");
        return 0;
    }
    /* header */
    buffer[idx++] = PROTCL_HEAD1;
    buffer[idx++] = PROTCL_HEAD2;
    /* mac */
    memcpy(buffer+idx, gCtx.uuid, sizeof(gCtx.uuid));
    idx += sizeof(gCtx.uuid);
    /* device type */
    buffer[idx++] = (DEVICE_TYPE >> 8);
    buffer[idx++] = (DEVICE_TYPE & 0xff);
    /* cmd */
    buffer[idx++] = cmd; 
    /* sequence */
    buffer[idx++] = seq >> 8; 
    buffer[idx++] = seq & 0xff; 
    /* data size */
    buffer[idx++] = dataLen;
    /* data */
    if (NULL != data && dataLen > 0)
    {
        if (dataLen > 1){
            memcpy(buffer+idx, data, dataLen);
        }else{
            buffer[idx] = data[0];
        }
        idx += dataLen;
    }
    /* crc */
    uint16_t crc = 0x00;

    crc = CRC16Cal(buffer + 2, idx - 2);
    buffer[idx++] = crc >> 8;
    buffer[idx++] = crc;

#if DEBUG
    LOG("Packet[%d]:",idx);
    int i;
    for(i = 0;i< idx;i++)
    {
        printf("[%02x]", buffer[i]);
    }
    printf("\n");
#endif

    return idx;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int parsePkt(uint8_t *cmd, uint16_t *seq, uint8_t *data, uint8_t dataSize,  uint8_t buffer[], uint8_t bufferSize)
{
    uint8_t idx = 0;
    uint8_t size;

#if DEBUG
    int i;
    LOG("Packet[%d]:", bufferSize);
    for (i = 0;i < bufferSize;i++)
    {
        printf("[%02x]", buffer[i]);
    }
    printf("\n");
#endif

    /* head check */
    if (buffer[idx++] != PROTCL_HEAD1) {
        LOG("Header Error\n");
        return -1;
    }
    if (buffer[idx++] != PROTCL_HEAD2) {
        LOG("Header Error\n");
        return -1;
    }

    /* crc check */
    uint16_t cal_crc = 0, pkt_crc = 0; 
    pkt_crc = buffer[bufferSize - 1] | buffer[bufferSize - 2] << 8;
    cal_crc = CRC16Cal(buffer + 2, bufferSize - 4);
#if 1
    if (pkt_crc != cal_crc) {
        LOGERROR("CRC Check Error, Cal:%04x, Pkt:%04x\n", cal_crc, pkt_crc);
        return -2;
    }
#endif
    LOGDEBUG(1, "CRC Cal:%02x, Pkt:%02x\n", cal_crc, pkt_crc);
    /* uuid match */
    if (memcmp(gCtx.uuid, buffer+idx, UUID_SIZE) != 0) {
        LOGERROR("MAC not Match\n");
        return -3;
    }
    idx += UUID_SIZE;

    /* device type match */
    uint16_t deviceType;

    deviceType = buffer[idx++] << 8;
    deviceType |= buffer[idx++] & 0xff;
    if (deviceType != DEVICE_TYPE) {
        LOGERROR("Device not Match\n");
        return -4;
    }

    /* cmd */
    *cmd = buffer[idx++];
    /* sequence */
    *seq = buffer[idx++] << 8;
    *seq |= buffer[idx++];

    /* data */
#if 1
    size = buffer[idx++];
    if (dataSize > size) {
        LOGERROR("Payload Buffer Size too Small.\n");
        return -5;
    }
    if (size > 1){
        memcpy(data, buffer + idx, size);
    }else if (1 == size){ 
        data[0] = buffer[idx];
        idx += size;
    }
#else
    payloadSize = buffer[idx++];
    *payloadPtrPtr = buffer + idx;
#endif

    return size;
}

/*  
 * @brief      serialize data     
 * @param[out] serial -- serial struct pointer 
 * @param[in]  frame -- frame data
 * @return     <=0 -- error; > 0 -- serial data
 * @see         
 * @note        
 */
int nblockProtclSerial(NblockFrame_t *frame, NblockSerial_t *serial)
{
    int retFun;
    char *uriPtr = NULL;
    int makePktLen;

    if (0 == gCtx.initd) {
        LOG("MAC Not Initialize\n");
        return -1;
    }

    retFun = cmdMatchUri(frame->cmd, &uriPtr);
    if (retFun != 0) {
        LOGERROR("URI Not Found\n");
        return 0;
    }

    /* parse uri */
    retFun = coapUriParseGetIp(uriPtr, serial->ip, sizeof(serial->ip));
    if (retFun < 0) {
        LOGERROR("Parse Uri Error\n");
        return 0;
    }
    serial->port = retFun;

    /* make packet  */
    retFun = makePkt(frame->cmd, frame->seq, frame->data, frame->dataLen, gTxRxBuffer, sizeof(gTxRxBuffer));
    if (retFun < 1) {
        LOGERROR("Make Packet Error\n");
        return 0;
    }
    makePktLen = retFun;

    /* coap serialize */
    retFun = coapClientSerial(gTxRxBuffer, makePktLen, COAP_TYPE_CON, COAP_POST, serial->buffer, sizeof(serial->buffer));
    if (retFun < 1) {
        LOGERROR("Coap Serialize Error\n");
        serial->bufferLen = 0;
        retFun = 0;
    }
    serial->bufferLen = retFun;
    return retFun;
}

/*  
 * @brief      parse receive message,NBIoT lock Prototol,Coap
 * @param[out] frame -- parsed information  
 * @param[in]  parse -- received message struct pointer 
 * @return     <0 -- error; > 0 -- payload size
 * @see         
 * @note        
 */
int nblockProtclParse(NblockFrame_t *frame, NblockParse_t *parse)
{
    int retFun;
    int parsedLen;

    if (0 == gCtx.initd) {
        LOG("MAC Not Initialize\n");
        return -1;
    }

    /* coap parse */
    retFun = coapClientParse(gTxRxBuffer, sizeof(gTxRxBuffer), parse->buffer, parse->bufferLen);
    if (retFun < 1) {
        LOG("Coap Parse Error\n");
        return -2;
    }
    parsedLen = retFun;
    /* parse packet */
    retFun = parsePkt(&frame->cmd, &frame->seq, frame->data, sizeof(frame->data), gTxRxBuffer, parsedLen);
    if (retFun < 0) {
        LOG("Parse Pakcet Error\n");
        return -3;
    }
    frame->dataLen = retFun;
    return retFun;
}


/*  
 * @brief      NBIoT lock protocol 整体初始化
 * @param[out] 
 * @param[in]  mac -- uuid 8 bytes, macSize -- mac length 
 * @return     None  
 * @see         
 * @note        
 */
int nblockProtclInit(const uint8_t mac[], uint16_t macSize)
{
    if (macSize != UUID_SIZE) {
        LOGERROR("MAC size[%d] not match\n", macSize);
        return -1;
    }
    memcpy(gCtx.uuid, mac, UUID_SIZE);
    gCtx.initd = 1;
    return 0;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
