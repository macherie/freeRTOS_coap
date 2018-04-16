/*  
 * @file     nblockProtcl.h	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018.03.27 
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef __NBPROTCLEXAM_H_
#define __NBPROTCLEXAM_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdint.h>

#include "nblockProtcl.h"


#define NBLOCK_RESPDATA_SIZE        NBLOCK_PROTCL_PAYLOAD_SIZE
#define APPPROTCL_DATASIZE          (64)
#define CMDSENDRECV_TIMEOUTMS       (10000)


/* up ping packet */
typedef struct 
{
    uint32_t time;
    uint8_t battery;
}Ping_t;

typedef struct
{
    uint8_t data;    
}General_t;

/* report attribute struct */
typedef struct
{
    uint8_t rpt_reason;
    uint16_t attrType;
    uint32_t attrIdx;
    uint32_t attrPos;
    uint32_t len;
    uint8_t data[APPPROTCL_DATASIZE];
}RptAttrReq_t;

/*Request OTA Data struct */
typedef struct
{
    uint8_t target;
    uint16_t sw_ver;
    uint32_t pos;
    uint32_t len;
}OtaDataReq_t;

/* response OTA Data Struct */
typedef struct
{
    uint8_t ret;
    uint8_t target;
    uint16_t sw_ver;
    uint32_t pos;
    uint32_t len;
    uint8_t data[APPPROTCL_DATASIZE];
}OtaDataRsp_t;

/* query Open Door Struct */
typedef struct
{
    uint8_t ascii[16];
    uint32_t time_start;
    uint32_t time_stop;
}LockOpenReq_t;

/* response Open Door Struct */
typedef struct
{
    uint8_t ret;        // 0x01 -- open door OK,0x00 -- open failed
    uint8_t reason;
}LockOpenRsp_t;

/* request OTA Start Struct */
typedef struct
{
    uint8_t target;
    uint8_t cmd;
    uint16_t sw_ver;
    uint32_t sw_sum;
    uint32_t sw_len;
}OtaStartReq_t;

/* response OTA Start Struct */
typedef struct
{
    uint8_t ret;
    uint8_t target;
}OtaStartRsp_t;

/* request Add Attribute */
typedef struct 
{
    uint16_t attrType;  
    uint32_t len;
    uint8_t data[APPPROTCL_DATASIZE];
}AddAttrReq_t;

/* request Delete Attribute */
typedef struct
{
    uint8_t attrType;
    uint8_t attrIdx;
}DelAttrReq_t;

/* response Add Delete Attribute */
typedef struct 
{
    uint8_t ret;
    uint16_t attrType;
    uint16_t attrIdx;
}AddDelAttrRsp_t;

/* request Mod Attribute */
typedef struct
{
    uint16_t attrType;
    uint32_t attrIdx;
    uint32_t attrPos;
    uint32_t len;
    uint8_t data[APPPROTCL_DATASIZE];
}ModAttrReq_t;

/* Response Mod Attribute */
typedef struct
{
    uint8_t ret;
    uint16_t attrType;
    uint16_t attrIdx;
    uint32_t pos;
    uint32_t len;
}ModAttrRsp_t;

/* request query Attribute */
typedef struct 
{
    uint16_t attrType;
    uint32_t attrStartIdx;
    uint32_t attrStartPos;
    uint32_t attrEndIdx;
    uint32_t attrEndPos;
    uint32_t len;
    uint16_t attrRptDlet;
}QryAttrReq_t;

/* response query Attribute */
typedef struct
{
    uint8_t ret;
    uint16_t attrType;
    uint32_t attrIdx;
    uint32_t attrPos;
    uint32_t len;
    uint8_t data[APPPROTCL_DATASIZE];
}QryAttrRsp_t;


int nblockProtcl_upJoin(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_upPing(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_upNetService(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_upRptAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_upOtaData(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspOpenLock(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspOtaStart(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspAddAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspDelAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspModAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int nblockProtcl_rspQryAttr(const void *dataPtr, NblockFrame_t *frame, uint16_t timeOut_ms);

int cmdAttachedFuncExe(uint8_t cmd, const void *dataPtr, NblockFrame_t *frame);


#ifdef __cplusplus 
} 
#endif

#endif /* nblockProtcl.h */
