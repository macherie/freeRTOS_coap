#include <stdio.h>
#include <stdarg.h>

#include "privatePrintf.h"
#include "stm32f1xx_hal.h"


static char printfbuffer[512];
static UART_HandleTypeDef UartHandle;


static void uart3Init(void)
{
  UartHandle.Instance        = USART3;

  UartHandle.Init.BaudRate   = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  
  HAL_UART_Init(&UartHandle);
}

static void putstring(char str[], unsigned short len)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)str, len, 0xFFFF);
}

void priPrintfInit(void)
{
    uart3Init();
}

int priPrintf(const char *format,...)
{
	int chars;
    
	va_list ap;
	
	va_start(ap, format);
	chars = vsnprintf(printfbuffer, sizeof(printfbuffer),format,ap);
	va_end(ap);
    putstring(printfbuffer, chars);
	
	return chars;
}