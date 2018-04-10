/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018.03.17
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#include "log.h"
#include "serial.h"
#include "myRingQueue.h"


#define UARTCOMM_DEBUG			(1)
#define NBIOT_UART_BAUDRATE		(9600)
#define RING_SIZE				(256)


typedef struct{
	int serialFd;
	ringQueue_t ringRecv;
}UartComm_ctx_t;


static UartComm_ctx_t gCtx;
static const char gSerialDev[] = "/dev/ttyUSB0";
static char gRingRecvBuffer[RING_SIZE];


/*  
 * @brief       open uart peripher 
 * @param[out]  None 
 * @param[in]  	None 
 * @return      >0 -- sucess; <=0 -- error 
 * @see         
 * @note       return value --> uart fd 
 */
static int uartInit(void)
{
	gCtx.serialFd = serial_open(gSerialDev, NBIOT_UART_BAUDRATE);
	return gCtx.serialFd;
}

/*  
 * @brief 		cyclay queue initialize       
 * @param[out]  rintPtr -- ring struct 
 * @param[in]   None 
 * @return      0 -- success;  
 * @see         
 * @note        
 */
static int ringRecvInit (ringQueue_t *ringPtr)
{
	initQueue(ringPtr, gRingRecvBuffer, sizeof(gRingRecvBuffer));
	return 0;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int uartComm_recv(void)
{
	uint8_t buffer[64];
	int retFun, idx;
	retFun = serial_read(gCtx.serialFd, buffer, sizeof(buffer), 0);
	if (retFun > 0){
		for(idx = 0;idx < retFun;idx++)
		{
			if (enQueue(&gCtx.ringRecv, buffer[idx]) == 0)
			{
				LOGERROR("Ring Full\n");	
				return idx;
			}
		}
	}else{
		retFun = 0;
	}
	return retFun;
}

/*  
 * @brief        get data from ring
 * @param[out]   data -- Receive Data Buffer 
 * @param[in]    size -- data size
 * @return       0 -- no data;>0 have data 
 * @see         
 * @note         
 */
int uartComm_getData(uint8_t data[], uint16_t size)
{
	int getLen = 0, retVal = 0, idx = 0;
	uint8_t temp;
	
	getLen = lenQueue(&gCtx.ringRecv);
	if (getLen > 0)
	{
		for(idx = 0;idx < getLen && idx < size;idx++)
		{
			popQueue(&gCtx.ringRecv, &temp);		/* get data from ring byte by byte */
			data[idx] = temp;
		}
		retVal = idx;
	}
	
	return retVal;
}

/*  
 * @brief        uart send data
 * @param[out]   None 
 * @param[in]    data -- send data buffer; size -- data size
 * @return       0 -- fail; >0 data size 
 * @see         
 * @note        
 */
int uartComm_sendData(const uint8_t data[], uint16_t size)
{
	int retFun;

#if (UARTCOMM_DEBUG == 1)
	char temp[128];
    int printLen;

	printLen = snprintf(temp,sizeof(temp),"%s",data);
	LOGDEBUG(1, "Send[%d]:%s\n",printLen, temp);

#endif
	retFun = serial_write(gCtx.serialFd, (const char *)data, size, 10); 
	if (retFun < 0)
	{
		retFun = 0;
	}
	return retFun;
}

/*  
 * @brief        uartComm initialize
 * @param[out]   None 
 * @param[in]    None 
 * @return       0 -- OK; other -- failed 
 * @see         
 * @note        
 */
int uartComm_init(void)
{
	int  retFunc, retVal = 1;
	retFunc = uartInit();
	if (retFunc > 0)
	{
		ringRecvInit(&gCtx.ringRecv);
		retVal = 0;
	}
	LOGDEBUG(UARTCOMM_DEBUG,"UART:%d\n", retFunc);

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
void uartComm_test()
{
	uartComm_init();
}




