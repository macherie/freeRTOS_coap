/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _BC95MODULE_H_
#define _BC95MODULE_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


#include <stdint.h>


#define APN_CMCC                ("CMNET")
#define DEFAULT_APN             APN_CMCC


typedef enum{
	ATCMD_OK,
	ATCMD_OUTTIME,
	ATCMD_ERROR,
    ATCMD_BUSY
}CMDStatus_t;


CMDStatus_t bc95_rest(void);

CMDStatus_t bc95_isRegist(void);

CMDStatus_t bc95_noEcho(void);

CMDStatus_t bc95_cgdcontSet(uint8_t cid, const char pdpType[], const char apn[]);

CMDStatus_t bc95_cgpaddr(uint8_t cid);

CMDStatus_t bc95_cgact(uint8_t cid, uint8_t state);

int bc95_serialNum(uint8_t serialNum[], uint16_t size);

int bc95_socket(const char ip[], uint16_t port);

int bc95_sendMsg(int socket, const char ip[], uint16_t port, const  uint8_t msg[], uint16_t msgLen);

int bc95_recvMsgPolling(uint8_t socket, uint8_t msg[], uint16_t msgSize, uint32_t timeOut_ms);

void nbiotBC95_test(void);


#ifdef __cplusplus 
} 
#endif

#endif /* bc95Module.h */
