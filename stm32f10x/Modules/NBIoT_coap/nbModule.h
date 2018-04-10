/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _NBPROTCL_H_
#define _NBPROTCL_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


#include "nblockProtcl.h"


typedef void (*CmdHandlerFunc_t)(uint8_t cmd, const uint8_t data[], uint16_t dataLen);    /* 命令处理函数 */
typedef void (*CmdErrHandlerFunc_t)(uint8_t cmd);                                       /* 错误处理函数 */


int nbInit(CmdHandlerFunc_t cmdHandlerFunc, CmdErrHandlerFunc_t errorHandlerFunc);

int nbMsgSend(uint8_t cmd, const uint8_t data[], uint16_t dataLen);

int nbStatus(void);

void nbRestart(void);




#ifdef __cplusplus 
} 
#endif

#endif /* nbProtcl.h */
