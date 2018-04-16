/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _COAP_CLIENT_H_
#define _COAP_CLIENT_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdint.h>

#include "er-coap-13.h"


int coapUriParseGetIp(const char uriStr[], char ip[], uint16_t ipSize);

int coapClientSerial(uint8_t payload[], uint16_t payloadSize, coap_message_type_t coapType, int method, uint8_t msgBuffer[], uint16_t msgSize);

int coapClientParse(uint8_t payload[], uint16_t payloadSize, uint8_t msgBuffer[], uint16_t msgSize);


#ifdef __cplusplus 
} 
#endif

#endif /* coap_client.h */
