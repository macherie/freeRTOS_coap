/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _COAP_UART_H_
#define _COAP_UART_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


void coapUartInit(void);

int coapUartSend(uint8_t data[], uint16_t dataLen);

int coapUartRecv(uint8_t data[], uint16_t len);


#ifdef __cplusplus 
} 
#endif

#endif /* coap_uart.h */
