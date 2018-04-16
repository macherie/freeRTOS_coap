/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
 

#include "stm32f1xx_hal.h"


#define UARTX_COAP		(USART1)
 
 
static UART_HandleTypeDef UartHandle;


/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
void coapUartInit(uint32_t baud)
{
	UartHandle.Instance        = UARTX_COAP;

	UartHandle.Init.BaudRate   = baud;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;
  
	HAL_UART_Init(&UartHandle);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int coapUartSend(uint8_t data[], uint16_t dataLen)
{
    int retVal = dataLen;
    
	if (HAL_UART_Transmit(&UartHandle, data, dataLen, 0xffff) != HAL_OK)
	{
		return 0;
	}
    return retVal;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int coapUartRecv(uint8_t data[], uint16_t size)
{
	if (HAL_UART_Receive(&UartHandle, data, size, 0xf) != HAL_OK)
	{
		return 0;
	}
	return 1;
}

