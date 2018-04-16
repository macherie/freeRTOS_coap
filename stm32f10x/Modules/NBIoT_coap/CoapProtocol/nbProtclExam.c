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
#include "nbiotCtrl.h"
#include "nblockProtcl.h"
#include "coap_client.h"
#include "nbProtclExam.h"


#define DEBUG 1

#define SW16(data)    (data >> 8)&0xff | (data << 8)&0xff00
#define SW32(data)    (data >> 24) & 0xff | (data >> 8) & 0xff00 | (data << 8 & 0xff0000) | (data << 24 & 0xff000000)
#define FILL2WORD(buffer, idx, sw, ptr) do{sw = SW32(ptr);\
                                           memcpy(buffer+idx, &sw, sizeof(ptr));\
                                           idx += sizeof(ptr);}while(0)
#define FILL1WORD(buffer, idx, sw, ptr) do{sw = SW16(ptr);\
                                           memcpy(buffer+idx, &sw, sizeof(ptr));\
                                           idx += sizeof(ptr);}while(0)
#define MEMCPY(buffer, idx, ptr, dataLen)    do{if (dataLen > 1){memcpy(buffer+idx,ptr,dataLen);}\
                                                else if (1 == dataLen){buffer[idx] = ptr[0];}\
                                                idx+=dataLen;\
                                                }while(0)

#define PERMIT_ID           "dusun"
#define PERMIT_PASSWD       "dusun"

#define nblockReqStructInput(cmdID, sequence, payload, payloadSize, req)    do{req->cmd=cmdID;req->seq=sequence;\
                                                                               if (payloadSize > 1)memcpy(req->data,payload, payloadSize);\
                                                                               else if (1 == payloadSize) req->data[0] = payload[0];\
                                                                               req->dataLen=payloadSize;\
                                                                              }while(0)


typedef int (*LockProtclExeFunc_t)(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

/* cmd attached function */
typedef struct 
{
    NbiotLockCmd_t cmd;
    LockProtclExeFunc_t exeFunc;
}CmdAttachedFunc_t;

static NblockSerial_t gSerial;
static NblockParse_t  gParse;


/*  
 * @brief     
 * @param[out] 
 * @param[in] 
 * @return     <=0 -- error;>0 send size
 * @see         
 * @note        
 */
static int dataSend(NblockSerial_t *send)
{
    int retFun;
#if DEBUG
    int i;
    LOG("Send[%d]:", send->bufferLen);
    for (i = 0;i < send->bufferLen;i++)
    {
        printf("[%02x]", send->buffer[i]);
    }
    printf("\n");
#endif
    retFun = nbIoTSendMsg(send->buffer, send->bufferLen);
    return retFun;
}

/*  
 * @brief     
 * @param[out] 
 * @param[in] 
 * @return     <0 -- error;0 -- timeout;>0 -- receive size
 * @see         
 * @note        
 */
static int dataRecv(NblockParse_t *recv, uint16_t timeOut_ms)
{
    int retFun = 0;
    timeOut_ms = timeOut_ms?timeOut_ms:1;

    retFun = nbIoTRecvMsg(recv->buffer, sizeof(recv->buffer), timeOut_ms);
    if (retFun > 0){
        recv->bufferLen = retFun;
        return retFun;
    }
    recv->bufferLen = 0;
    LOG("Recieve Time out Or Error\n");
    return -2;
}

/*  
 * @brief      make packet-->send-->receive-->parse packet
 * @param[out] framePtr -- NBIoT lock protocol response pointer 
 * @param[in]  framePtr -- request point; timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
static int  transRecvHandlePkt(NblockFrame_t *framePtr, uint16_t timeOut_ms)
{
    //uint16_t recvPktLen;
    int retFun;


    /* serial */
    retFun = nblockProtclSerial(framePtr, &gSerial);
    if (retFun < 1) {
        LOG("Serial Error\n");
        return -1;
    }

    /* Set Peer IP, Port */
    nbIoTSetPeer(gSerial.ip, gSerial.port);

    /* send */
    retFun = dataSend(&gSerial);
    if (retFun < 1){
        LOG("Send Error\n");
        return -2;
    }

    /* receive */
    retFun = dataRecv(&gParse, timeOut_ms);
    if (retFun < 1)
    {
        LOG("Receive Error or time out\n");
        return -3;
    }
    
    /* parse */
    retFun = nblockProtclParse(framePtr,  &gParse);
    if (retFun < 0)
    {
        LOG("Parse Error\n");
        return -4;
    }
    return 0;
}

/*  
 * @brief      Request to join network    
 * @param[out] rspPtr -- NblockProtcl Response Struct  
 * @param[in]  timeOut_ms -- timeout 
 * @return     0 -- Success;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_upJoin(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    int retFun;
    char temp[64];
    uint16_t tempIdx;

    LOGDEBUG(DEBUG, "Join Netwok\n");

    tempIdx = sprintf(temp, "%s",PERMIT_ID);
    temp[tempIdx++] = '\0';
    tempIdx += sprintf(temp + tempIdx, "%s",PERMIT_PASSWD);
    temp[tempIdx++] = '\0';

    nblockReqStructInput(NBLOCK_UPCMD_PERMIT, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);
    return retFun;
}

/*  
 * @brief      Request Ping to Server  
 * @param[out] rspPtr -- response struct pointer  
 * @param[in]  infoPtr -- Device Info struct pointer; timeOut_ms -- timeout 
 * @return     0 -- Response Ok;other -- Response error  
 * @see         
 * @note        
 */
int nblockProtcl_upPing(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Ping \n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    int retFun;
    uint8_t temp[8];
    uint16_t tempIdx = 0;
    uint32_t bigEndianVal32;
    Ping_t *ptr = (Ping_t *)dataPtr;

    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->time);
    temp[tempIdx++] = ptr->battery;

    nblockReqStructInput(NBLOCK_PING, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);

    return retFun;

}

/*  
 * @brief      Request Net Service Stop or Start   
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_upNetService(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Net Service \n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    int retFun;
    uint8_t temp[8];
    uint16_t tempIdx = 0;
    General_t *ptr = (General_t *)dataPtr;

    temp[tempIdx++] = ptr->data;

    nblockReqStructInput(NBLOCK_UPCMD_NETSERVICE, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);
    return retFun;
}

/*  
 * @brief      Report Attibutes
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_upRptAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Report Attribute\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    int retFun;
    uint8_t temp[NBLOCK_PROTCL_PAYLOAD_SIZE];
    uint8_t tempIdx = 0;
    uint16_t bigEndianVal16;
    uint32_t bigEndianVal32;
    RptAttrReq_t *ptr = (RptAttrReq_t *)dataPtr;

    temp[tempIdx++] = ptr->rpt_reason;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrType);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->attrIdx);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->attrPos);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->len);
    if (ptr->len > 1) {
        memcpy(temp + tempIdx, ptr->data, ptr->len);
    }else if (1 == ptr->len){
        temp[tempIdx] = ptr->data[0];
    }
    tempIdx += ptr->len;

    nblockReqStructInput(NBLOCK_UPCMD_RPTATTR, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);
    return retFun;
}

/*  
 * @brief      Request OTA Data
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_upOtaData(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "OTA Data\n");

    int retFun;
    uint8_t temp[64];
    uint8_t tempIdx = 0;
    uint16_t bigEndianVal16;
    uint32_t bigEndianVal32;
    OtaDataReq_t *ptr = (OtaDataReq_t *)dataPtr;

    temp[tempIdx++] = ptr->target;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->sw_ver);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->pos);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->len);

    nblockReqStructInput(NBLOCK_UPCMD_OTADATA, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);
    return retFun;
}

/*  
 * @brief      general single data send
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
#if 0
static int general_oneDataCmd(uint8_t cmd, uint8_t data, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    int retFun;
    uint8_t temp[1];
    uint8_t tempIdx = 0;

    temp[tempIdx++] = data;

    nblockReqStructInput(cmd, 0, temp, tempIdx, frame);
    retFun = transRecvHandlePkt(frame, timeOut_ms);
    return retFun;
}
#endif

/*  
 * @brief      Response for open lock CMD
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_rspOpenLock(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For Open Lock.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[32];
    uint16_t tempIdx = 0;
    LockOpenRsp_t *ptr = (LockOpenRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    temp[tempIdx++] = ptr->reason;
    nblockReqStructInput(NBLOCK_DOWNCMD_OPENLOCK, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);
}

/*  
 * @brief      Response for query Version CMD
 * @param[out] frame -- Response Struct;  
 * @param[in]  dataPtr -- request data struct; 
 *             timeOut_ms -- timeout
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int nblockProtcl_rspOtaStart(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For OTA Start.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[32];
    uint16_t tempIdx = 0;
    OtaStartRsp_t *ptr = (OtaStartRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    temp[tempIdx++] = ptr->target;
    nblockReqStructInput(NBLOCK_DOWNCMD_OTASTART, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nblockProtcl_rspAddAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For Add Attribute.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[32];
    uint16_t tempIdx = 0;
    uint16_t bigEndianVal16;
    AddDelAttrRsp_t *ptr = (AddDelAttrRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrType);
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrIdx);
    

    nblockReqStructInput(NBLOCK_DOWNCMD_ADDATTR, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);

}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nblockProtcl_rspDelAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For Delete Attribute.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[32];
    uint16_t tempIdx = 0;
    uint16_t bigEndianVal16;
    AddDelAttrRsp_t *ptr = (AddDelAttrRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrType);
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrIdx);
    
    nblockReqStructInput(NBLOCK_DOWNCMD_DELATTR, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nblockProtcl_rspModAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For Add Attribute.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[32];
    uint16_t tempIdx = 0;
    uint16_t bigEndianVal16;
    uint32_t bigEndianVal32;
    ModAttrRsp_t *ptr = (ModAttrRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrType);
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrIdx);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->pos);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->len);

    nblockReqStructInput(NBLOCK_DOWNCMD_MODATTR, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);

}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nblockProtcl_rspQryAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms)
{
    LOGDEBUG(DEBUG, "Response For Add Attribute.\n");
    if (NULL == dataPtr) {
        LOGERROR("Request Data Empty\n");
        return 1;
    }

    uint8_t temp[100];
    uint16_t tempIdx = 0;
    uint16_t bigEndianVal16;
    uint32_t bigEndianVal32;
    QryAttrRsp_t *ptr = (QryAttrRsp_t *)dataPtr;

    temp[tempIdx++] = ptr->ret;
    FILL1WORD(temp, tempIdx, bigEndianVal16, ptr->attrType);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->attrIdx);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->attrPos);
    FILL2WORD(temp, tempIdx, bigEndianVal32, ptr->len);
    MEMCPY(temp, tempIdx, ptr->data, ptr->len);
    
    nblockReqStructInput(NBLOCK_DOWNCMD_QRYATTR, 0, temp, tempIdx, frame);
    return transRecvHandlePkt(frame, timeOut_ms);

}



static CmdAttachedFunc_t gAttachedFuncs[]={
    {NBLOCK_PING,               nblockProtcl_upPing},
    {NBLOCK_UPCMD_PERMIT,       nblockProtcl_upJoin},
    {NBLOCK_UPCMD_NETSERVICE,   nblockProtcl_upNetService},
    {NBLOCK_UPCMD_RPTATTR,      nblockProtcl_upRptAttr},
    {NBLOCK_UPCMD_OTADATA,      nblockProtcl_upOtaData},
    {NBLOCK_DOWNCMD_OPENLOCK,   nblockProtcl_rspOpenLock},
    {NBLOCK_DOWNCMD_ADDATTR,    nblockProtcl_rspAddAttr},
    {NBLOCK_DOWNCMD_DELATTR,    nblockProtcl_rspDelAttr},
    {NBLOCK_DOWNCMD_MODATTR,    nblockProtcl_rspModAttr},
    {NBLOCK_DOWNCMD_QRYATTR,    nblockProtcl_rspQryAttr},
    {NBLOCK_DOWNCMD_OTASTART,   nblockProtcl_rspOtaStart}
};


/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int cmdAttachedFuncExe(uint8_t cmd, const void *dataPtr, NblockFrame_t *frame)
{
    uint8_t idx;
    int retFunc;

    for (idx = 0; idx < sizeof(gAttachedFuncs)/sizeof(gAttachedFuncs[0]);idx++){
        if (gAttachedFuncs[idx].cmd == cmd){
            retFunc = gAttachedFuncs[idx].exeFunc(dataPtr, frame, CMDSENDRECV_TIMEOUTMS);
            return retFunc;
        }
    }
    LOG("CMD No Matched Function\n");
    return -10;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */

