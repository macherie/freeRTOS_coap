/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _UARTCOMM_H_
#define _UARTCOMM_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


#include <stdint.h>


int uartComm_getData(uint8_t data[], uint16_t size);

int uartComm_sendData(uint8_t data[], uint16_t size);

int uartComm_recv(void);

int uartComm_init(void);


#ifdef __cplusplus 
} 
#endif

#endif /* uartComm.h */
