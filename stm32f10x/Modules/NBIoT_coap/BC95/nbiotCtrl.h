/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _NBIOTCTRL_H_
#define _NBIOTCTRL_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdint.h>


typedef enum{
    NBCTRL_NOTINITD = 0,
    NBCTRL_CLOSED=1,
    NBCTRL_CONNECTING,
    NBCTRL_CONNECTED,
    NBCTRL_SENDING,
    NBCTRL_SENDDONE,
    NBCTRL_RECVING,
    NBCTRL_RECVDONE,
    NBCTRL_NOCONNECTED,
}NbCtrlStatus_t;


int nbIoTRestart(void);

int nbIoTInit(const char ip[], uint16_t port);

int nbIoTSendMsg(const uint8_t data[], uint16_t dataLen);

int nbIoTRecvMsg(uint8_t data[], uint16_t dataSize, uint32_t timeOut);

NbCtrlStatus_t nbIoTGetState(void);

void nbIoTSetState(NbCtrlStatus_t expect);

int nbIoTSetPeer(const char ip[], uint16_t port);


#ifdef __cplusplus 
} 
#endif

#endif /* nbiotCtrl.h */
