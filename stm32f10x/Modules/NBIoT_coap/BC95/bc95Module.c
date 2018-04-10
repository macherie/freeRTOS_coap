/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <string.h>
#include <stdio.h>

#include "log.h"
#include "util.h"
#include "uartComm.h"
#include "bc95Module.h"


#define BC95MODULE_DEBUG		(1)
#define BC95TXRX_BUFFER_SIZE    (512)
#define CMD_SIZE				(64)

/* BC95 AT CMD */
#define BC95_NRB				("NRB")			    /* reset bc95 module */
#define BC95_ATE0				("ATE0")            /* No Echo */
#define BC95_CGSN               ("CGSN")            /* Get IMEI Number */
#define BC95_CEREG				("CEREG")			/* check bc95 module registed */
#define BC95_CFUN				("CFUN")			/* now bc95 module functional mode */
#define BC95_CGPADDR			("CGPADDR")		    /* after bc95 module registed, show IP addr */
#define BC95_CGDCONT			("CGDCONT")	        /* set PDP Content */
#define BC95_CGACT				("CGACT")           /* active cid PDP Content */
#define BC95_NSOCR              ("NSOCR")           /* Create socket */
#define BC95_NSOST              ("NSOST")           /* socket send message */
#define BC95_NSORF              ("NSORF")           /* socket receive message */
#define BC95_NSOCL              ("NSOCL")           /* Close socket */

/* cmd feedback */
#define BC95_FEDBACK_OK         ("OK\r")
#define BC95_FEDBACK_ERROR      ("ERROR\r")
#define BC95_NSONMI             ("NSONMI:")
#define RECV_HEADER_SIZE        (40)



typedef struct 
{
    int sockFd;
	char serverIP[64];
	uint16_t port;
    uint8_t sem;            /* semaphore */
}NbiotCtx_t;


static NbiotCtx_t gCtx;
static uint8_t gTxRxBuffer[BC95TXRX_BUFFER_SIZE];


/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int bc95StringSend(const char str[])
{
	int size = strlen(str);	

	return uartComm_sendData((const uint8_t *)str, size);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int bc95StringRecv(char data[], uint16_t size)
{
	int recvLen, retFun = 0; 

	recvLen = uartComm_recv();
	if (recvLen > 0) {
		retFun = uartComm_getData(data, recvLen);
	}
    return retFun;
}

/*  
 * @brief      get expect string  
 * @param[out] startIdx -- expect String index in Souce; endIdx -- expect String index in Source;
 * @param[in]  src -- source;dotCnt -- dot Counter  
 * @return     < 0 -- error; > 0 -- startIdx Success
 * @see         
 * @note        
 */
static int getStringbyDot(const char src[], uint8_t dotNum, uint16_t *startIdx, uint16_t *endIdx)
{
    int retVal = -1;
    const char *ptr = src;
    char dot = ',';
    uint8_t dotCnt = 0;

    while (*ptr != '\0'){
        if (*ptr == dot) {
            if (++dotCnt == dotNum) {
                *startIdx = ptr - src + 1;
                retVal = *startIdx;
                ptr++;
                break;
            }
        }
        ptr++;
    }
    while (*ptr != '\r' && *ptr != '\0' && *ptr != '\n' && *ptr != dot){
        ptr++;
    }
    *endIdx = ptr - src;
    return retVal;
}

/*  
 * @brief      get expect string  
 * @param[out] startIdx -- expect String index in Souce; endIdx -- expect String index in Source;
 * @param[in]  src -- source;dotCnt -- dot Counter  
 * @return     < 0 -- error; > 0 -- startIdx Success
 * @see         
 * @note        
 */
static int getStringbyColon(const char src[], uint8_t dotNum, uint16_t *startIdx, uint16_t *endIdx)
{
    int retVal = -1;
    const char *ptr = src;
    char dot = ':';
    uint8_t dotCnt = 0;

    while (*ptr != '\0'){
        if (*ptr == dot) {
            if (++dotCnt == dotNum) {
                *startIdx = ptr - src + 1;
                retVal = *startIdx;
                ptr++;
                break;
            }
        }
        ptr++;
    }
    while (*ptr != '\r' && *ptr != '\0' && *ptr != '\n' && *ptr != dot){
        ptr++;
    }
    *endIdx = ptr - src;
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
static CMDStatus_t recv_fedback(uint32_t outtime_ms)
{
    int retFun, recvdLen;

    for(recvdLen = 0;outtime_ms > 0;outtime_ms--)
	{
		retFun = bc95StringRecv(gTxRxBuffer + recvdLen, sizeof(gTxRxBuffer) - recvdLen - 1);
		recvdLen += retFun;
		if (bfCmp(gTxRxBuffer, recvdLen, BC95_FEDBACK_OK,strlen(BC95_FEDBACK_OK)) != -1)
		{
			gTxRxBuffer[recvdLen]='\0';
			LOGDEBUG(BC95MODULE_DEBUG, "Matched\n");
			LOGDEBUG(BC95MODULE_DEBUG, "Recv[%d]:%s.\n",recvdLen,gTxRxBuffer);

			return ATCMD_OK;
		}else if (bfCmp(gTxRxBuffer, recvdLen,BC95_FEDBACK_ERROR,strlen(BC95_FEDBACK_ERROR)) != -1) {
            gTxRxBuffer[recvdLen] = '\0';
			LOGDEBUG(BC95MODULE_DEBUG, "Matched\n");
            LOGDEBUG(BC95MODULE_DEBUG,"Recv[%d]:%s.\n", recvdLen, gTxRxBuffer);

            return ATCMD_ERROR;
        }
        nb_delay(1);
	}
	gTxRxBuffer[recvdLen]='\0';
	LOGDEBUG(BC95MODULE_DEBUG, "No Matched.\n");
	LOGDEBUG(BC95MODULE_DEBUG, "Recv[%d]:%s\n",recvdLen,gTxRxBuffer);

    return ATCMD_OUTTIME;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static CMDStatus_t msgArrived(uint32_t outtime_ms)
{
    int retFun, recvdLen;

    for(recvdLen = 0;outtime_ms > 0;outtime_ms--)
	{
		retFun = bc95StringRecv(gTxRxBuffer + recvdLen, sizeof(gTxRxBuffer) - recvdLen - 1);
		recvdLen += retFun;
		if (bfCmp(gTxRxBuffer, recvdLen, BC95_NSONMI,strlen(BC95_NSONMI)) != -1) {

            if (bfCmp(gTxRxBuffer + 2, recvdLen,"\r\n", strlen("\r\n")) != -1) {
			    gTxRxBuffer[recvdLen]='\0';
			    LOGDEBUG(BC95MODULE_DEBUG, "Matched\n");
			    LOGDEBUG(BC95MODULE_DEBUG, "Recv[%d]:%s.\n",recvdLen,gTxRxBuffer);
			    return ATCMD_OK;
            }
        }
        nb_delay(1);
	}
	gTxRxBuffer[recvdLen]='\0';
	LOGDEBUG(BC95MODULE_DEBUG, "No Matched.\n");
	LOGDEBUG(BC95MODULE_DEBUG, "Recv[%d]:%s\n",recvdLen,gTxRxBuffer);

    return ATCMD_OUTTIME;
}


/*  
 * @brief      at cmd send and receive feedback message
 * @param[out] None  
 * @param[in]  cmd -- at cmd;outtime_ms -- outtime 
 * @return     CMDStatus_t  
 * @see         
 * @note       after send cmd, check recieve message is ok or error. 
 */
static CMDStatus_t atcmd_fedback(const char cmd[], uint32_t outtime_ms)
{
	CMDStatus_t retVal = ATCMD_ERROR;
	int retFun, recvdLen;

	/* send data */
	retFun = bc95StringSend(cmd);
	if (retFun < 1)
	{
		LOGERROR("Send Data Error\n");
		return retVal;
	}
	/* recv data expect OK\r */
    return recv_fedback(outtime_ms);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int msgReceive(uint8_t socket, uint16_t recvLen)
{
	char cmdBuffer[CMD_SIZE];
    int retVal;
    CMDStatus_t retFun = ATCMD_ERROR;

    /* Receive Message */
	snprintf(cmdBuffer, sizeof(cmdBuffer),"AT+%s=%u,%u\r",BC95_NSORF, socket, recvLen);
	retFun = atcmd_fedback(cmdBuffer, 500);
    if (ATCMD_OK != retFun) {
        LOG("Receive Error\n");
        return 0;
    }
    /* Received Length */
    uint16_t start, end;
    uint32_t recvdLen;
    retVal = getStringbyDot(gTxRxBuffer, 3, &start, &end);
    if (retVal < 0) {
        LOG("Get String by Dot Error.\n");
        return 0;
    }
    retVal = strdec2dec_uint32(gTxRxBuffer + start, end - start, &recvdLen);
    if (retVal != 0){
        LOG("String to Hec Error\n");
        return 0;
    }
    return recvdLen;
}

/*  
 * @brief      at cmd send and receive message check it
 * @param[out] dataBuffer -- payload data  
 * @param[in]  cmd -- at cmd;timeout_ms -- receive message deadline 
 *             size -- payload data buffer
 * @return     CMDStatus_t   
 * @see         
 * @note        
 */
static CMDStatus_t atcmd_result(const char cmd[], uint32_t timeout_ms, char dataBuffer[], uint16_t size)
{
    CMDStatus_t retVal = ATCMD_ERROR;
	int retFun, recvdLen, idx;

    /* send data */
	retFun = bc95StringSend(cmd);
	if (retFun < 1)
	{
		LOGERROR("Send Data Error\n");
		return retVal;
	}

    /* receive data */
    retVal = recv_fedback(timeout_ms);
    if (ATCMD_OK != retVal)
    {
        LOGDEBUG(BC95MODULE_DEBUG,"Receive Data Error:%d\n", retVal);    
        return retVal;
    }

    /* get data */
    retFun = snprintf(dataBuffer,size,"%s", gTxRxBuffer);
    dataBuffer[retFun - 1] = '\0';

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
static CMDStatus_t atcmd_resultIsMatched(const char cmd[], uint32_t timeout_ms, char expectStr[][CMD_SIZE], uint16_t num)
{
    CMDStatus_t retVal = ATCMD_ERROR;
	int retFun, recvdLen, idx;

	/* send data */
	retFun = bc95StringSend(cmd);
	if (retFun < 1)
	{
		LOGERROR("Send Data Error\n");
		return retVal;
	}

    /* receive data */
    retVal = recv_fedback(timeout_ms);
    if (ATCMD_OK != retVal)
    {
        LOGDEBUG(BC95MODULE_DEBUG,"Receive Data Error:%d\n", retVal);    
        return retVal;
    }

    /* parse data */
    for (idx = 0;idx < num;idx++)
    {
        retFun = bfCmp(gTxRxBuffer, strlen(gTxRxBuffer), expectStr[idx],strlen(expectStr[idx]));
        if (retFun != -1)
        {
            LOGDEBUG(BC95MODULE_DEBUG,"Result Matched\n");
            retVal = ATCMD_OK;
            return retVal;
        }
    }
    retVal = ATCMD_ERROR; 
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
static CMDStatus_t nbiot_moduleCMD(const char cmd[], uint32_t timeout_ms)
{
	char cmdBuffer[CMD_SIZE];
	CMDStatus_t retVal = ATCMD_ERROR;

	snprintf(cmdBuffer, sizeof(cmdBuffer),"AT+%s\r",cmd);
	return atcmd_fedback(cmdBuffer, timeout_ms);
}


/*  
 * @brief       reset bc95  module 
 * @param[out]  None 
 * @param[in]   None 
 * @return       CMDStatus_t:ATCMD_OK -- success;
 *				 ATCMD_ERROR -- error; ATCMD_OUTTIME -- timeout; 
 * @see         
 * @note        
 */
CMDStatus_t bc95_rest(void)
{
	return nbiot_moduleCMD(BC95_NRB, 15000);
}

/*  
 * @brief        set bc95 module no input echo
 * @param[out]   None 
 * @param[in]    None 
 * @return       CMDStatus_t:ATCMD_OK -- success;
 *				 ATCMD_ERROR -- error; ATCMD_OUTTIME -- timeout; 
 * @see         
 * @note        
 */
CMDStatus_t bc95_noEcho(void)
{
	char cmdBuffer[CMD_SIZE];
	CMDStatus_t retVal = ATCMD_ERROR;

	snprintf(cmdBuffer,sizeof(cmdBuffer), "%s\r", BC95_ATE0);

	return atcmd_fedback(cmdBuffer, 500);
}

/*  
 * @brief       is bc95 registed on network 
 * @param[out]  None 
 * @param[in]   None 
 * @return      CMDStatus_t:ATCMD_OK -- success;
 *			    ATCMD_ERROR -- error; ATCMD_OUTTIME -- timeout; 
 * @see         bc95 AT commander pdf
 * @note       	
 */
CMDStatus_t bc95_isRegist(void)
{
	char cmdBuffer[CMD_SIZE];
	char expectStr[2][CMD_SIZE] = {"0,1",
								   "0,4"};
	CMDStatus_t retFun;

	snprintf(cmdBuffer, sizeof(cmdBuffer),"AT+%s?\r", BC95_CEREG);
	retFun = atcmd_resultIsMatched(cmdBuffer, 1000, expectStr, 2);
	return retFun;
}


/*  
 * @brief      set PDP Context 
 * @param[out] None  
 * @param[in]  cid -- specifies  a  particular  PDP  context  definition
 *     		   pdpType -- specifies the type of packet data protocol
 *			   apn
 * @return     CMDStatus_t:
 *			   	ATCMD_OK 		-- success;
 *			   	ATCMD_ERROR 	-- error; 
 *				ATCMD_OUTTIME 	-- timeout; 
 * @see        bc95 AT commander pdf
 * @note        
 */
CMDStatus_t bc95_cgdcontSet(uint8_t cid, const char pdpType[], const char apn[])
{
	char cmdBuffer[CMD_SIZE];
	CMDStatus_t retFun;

	snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+%s=%u,\"%s\",\"%s\"\r",BC95_CGDCONT, cid, pdpType,apn);
	retFun = atcmd_fedback(cmdBuffer, 500);
	return retFun;
}

/*  
 * @brief       PDP Content Registed get IP address 
 * @param[out]  None 
 * @param[in]   cid -- PDP Content ID
 * @return       
 * @see         bc95 AT commander pdf
 * @note        
 */
CMDStatus_t bc95_cgpaddr(uint8_t cid)
{
	char cmdBuffer[CMD_SIZE];
	CMDStatus_t retFun;
	
	snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+%s=%u\r",BC95_CGPADDR, cid);
	retFun = atcmd_fedback(cmdBuffer, 500);
	return retFun;
}

/*  
 * @brief      Activate or Deactivate PDP Context  
 * @param[out] None 
 * @param[in]  cid -- specifies a particular PDP context definition  
 *			   state --  indicates the activation state of PDP context
 * @return     CMDStatus_t:
 *			   	ATCMD_OK 		-- success;
 *			   	ATCMD_ERROR 	-- error; 
 *				ATCMD_OUTTIME 	-- timeout; 
 * @see        bc95 AT commander pdf
 * @note        
 */
CMDStatus_t bc95_cgact(uint8_t cid, uint8_t state)
{
	char cmdBuffer[CMD_SIZE];
	CMDStatus_t retFun;
	
	state = state?1:0;
	snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+%s=%u,%u\r",BC95_CGACT, state, cid);
	retFun = atcmd_fedback(cmdBuffer, 1000);
	return retFun;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
*/
CMDStatus_t bc95_CGSN(uint8_t snt, char dataBuffer[], uint16_t dataSize)
{
    char cmdBuffer[CMD_SIZE];
    CMDStatus_t retFun;
    
    memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
    snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+%s=%u\r", BC95_CGSN, snt);
    retFun = atcmd_result(cmdBuffer, 500, dataBuffer, dataSize);
    if (ATCMD_OK != retFun) {
        LOG("Can't Recieve "); 
    }
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return    <0 -- error; >=0 -- OK 
 * @see         
 * @note        
 */
int bc95_socket(const char ip[], uint16_t port)
{
    char cmdBuffer[CMD_SIZE];
    char recvBuffer[CMD_SIZE];
    CMDStatus_t retFun;
    int retVal;

    snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+%s=DGRAM,17,%u\r", BC95_NSOCR,port);
    retFun = atcmd_result(cmdBuffer, 500, recvBuffer, sizeof(recvBuffer));
    if (retFun != ATCMD_OK)
    {
        LOGERROR("Create Socket Error\n");
        return -1;
    }
    /* turn socket fd */
    unsigned int sockFd;
    retVal = mysscan_uint(recvBuffer + 2, &sockFd);
    if (retVal < 0){
        LOG("Turn Socket Descriptor Error\n");
        return -1;
    }
    return sockFd;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int bc95_sendMsg(int socket, const char ip[], uint16_t port, const  uint8_t msg[], uint16_t msgLen)
{
    int bufferLen;
    int allBufferLen;
    int idx;
    uint8_t recvBuffer[CMD_SIZE];
    CMDStatus_t retFun;

    /* serialize */
    bufferLen = snprintf(gTxRxBuffer, sizeof(gTxRxBuffer), "AT+%s=%u,%s,%u,%u,", BC95_NSOST, socket, ip, port, msgLen);
    allBufferLen = bufferLen + msgLen * 2 + 2;
    if (allBufferLen > sizeof(gTxRxBuffer)){
        LOG("TxRxBuffer too Small\n");
        return 0;
    }

    for (idx = 0;idx < msgLen ;idx++) {
       bufferLen += snprintf(gTxRxBuffer + bufferLen, sizeof(gTxRxBuffer) - bufferLen,"%02X",msg[idx]); 
    }
    gTxRxBuffer[bufferLen++] = '\r';
    gTxRxBuffer[bufferLen++] = 0;

    /* send Message and Feedback */
    retFun = atcmd_result(gTxRxBuffer, 500, recvBuffer, sizeof(recvBuffer));
    if (ATCMD_OK != retFun) {
        LOG("Receive Error:%d\n", retFun);
        return 0;
    }
    uint16_t start, end;
    uint32_t value;
    int ret;
    ret = getStringbyDot(recvBuffer, 1, &start, &end);
    if (ret < 1){
        LOG("Get String by Dot Error\n");
        return 0;
    }
    ret = strdec2dec_uint32(recvBuffer+ start, end - start, &value);
    if (ret != 0){
        LOG("String to Hec Error\n");
        return 0;
    }
    LOGDEBUG(BC95MODULE_DEBUG, "Send Length:%d\n", value);
    return value;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int bc95_recvMsgPolling(uint8_t socket, uint8_t msg[], uint16_t msgSize, uint32_t timeOut_ms)
{
    CMDStatus_t retFun;
    int retVal;

    /* NSONMI */
    retFun = msgArrived(timeOut_ms);
    if (ATCMD_OK != retFun) {
        LOG("Received Arrived Message Error \n");
        return -1;
    }
    /* Arrived Message Length */
    uint16_t start, end;
    uint32_t value;
    int ret;
    ret = getStringbyDot(gTxRxBuffer, 1, &start, &end);
    if (ret < 1){
        LOG("Get String by Dot Error\n");
        return -2;
    }
    ret = strdec2dec_uint32(gTxRxBuffer + start, end - start, &value);
    if (ret != 0){
        LOG("String to Hec Error\n");
        return -4;
    }
    LOGDEBUG(BC95MODULE_DEBUG, "Received Length:%d\n", value);
    /* Received Message From NBIoT Module */
    if (value * 2 > BC95TXRX_BUFFER_SIZE - RECV_HEADER_SIZE) {
        LOG("Internel Receive Buffer too Small\n");
        return -5;
    }
    if (value > msgSize ) {
        msgReceive(socket, value);
        LOG("Message Buffer too Small\n");    
        return 0;
    }
    retVal = msgReceive(socket, value);
    if (retVal == 0) {
        LOG("Recieve None\n");
        return 0;
    }
    /* message turn hex array */
    uint8_t hex;
    uint16_t idx;
    ret = getStringbyDot(gTxRxBuffer, 4, &start, &end);
    if (ret < 1){
        LOG("Get String by Dot Error\n");
        return 0;
    }
    for (idx = 0;start < end;idx++, start += 2) {
        strhex2hex_byte(gTxRxBuffer + start, 2, &hex);
        msg[idx] = hex;
    }
    return idx;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int bc95_recvMsg()
{
    /* TODO */
    return 0;
}

/*  
 * @brief      get NBIoT Module MAC
 * @param[out] serialNum -- module mac buffer
 * @param[in]  size -- serialNum buffer size
 * @return     <=0 -- error; >0 -- OK, MAC length
 * @see         
 * @note        
 */
int bc95_serialNum(uint8_t serialNum[], uint16_t size)
{
    if (size < 8) {
        LOG("Output Array too Small\n");
        return 0;
    }
    CMDStatus_t retFun = ATCMD_ERROR;
    char recvBuffer[CMD_SIZE];
    int retVal;

    /* Send CMD */
    retFun = bc95_CGSN(1, recvBuffer, sizeof(recvBuffer)); 
    if (ATCMD_OK != retFun) {
        LOG("Can not Get Serial Number\n");
        return -1;
    }
    /* turn IMEI(dec) 15bytes to 8bytes(dec) */
    uint16_t start, end;
    uint8_t hex, idx;
    retVal = getStringbyColon(recvBuffer, 1, &start, &end);
    if (retVal < 0){
        LOG("Get String by Colon Error\n");
        return -2;
    }
    start--;
    if (end - start > size * 2)
    {
        LOG("Output Buffer too Small\n");
        return -3;
    }
    recvBuffer[start] = '0';
    for (idx = 0;start < end; start += 2, idx++) {
        strhex2hex_byte(recvBuffer + start, 2, &hex);
        serialNum[idx] = hex;
    }
    return idx;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static void nbiotBC95_process(void *para)
{
    int retFun;
    retFun = uartComm_init();
    if (retFun != 0)
    {
        LOG("Uart Initialize Error\n");
        while(1);
    }
    LOGDEBUG(BC95MODULE_DEBUG,"UART Initialize OK:%d\n", retFun);
}


/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
void nbiotBC95_test(void)
{
    CMDStatus_t retFun;
    int retVal;
    nbiotBC95_process(NULL);
    uint8_t sendBuffer[256];

    do{
        LOGDEBUG(BC95MODULE_DEBUG,"Reset BC95 Module\n");
        retFun = bc95_rest();
    }while(ATCMD_OK != retFun);

    bc95_noEcho();

    retVal = bc95_serialNum(sendBuffer, 16);
    if (retVal < 0) {
        LOG("Can't Get BC95 Serial Number\n");
        return;
    }

    LOG("Serial Number:%02x %02x %02x %02x\n", sendBuffer[0], sendBuffer[1],sendBuffer[2], sendBuffer[3]);

    do{
        LOGDEBUG(BC95MODULE_DEBUG,"Check BC95 Is Registed\n");
        retFun = bc95_isRegist();
        if (ATCMD_OK != retFun)
        {
            nb_delay(2000);
        }
    }while(ATCMD_OK != retFun);

    do{
        LOGDEBUG(BC95MODULE_DEBUG, "cgdcont Set\n");
        retFun = bc95_cgdcontSet(1, "IP", DEFAULT_APN);
        if (ATCMD_OK != retFun)
        {
            nb_delay(1000);
        }
    }while(ATCMD_OK != retFun);
    
    do{
        LOGDEBUG(BC95MODULE_DEBUG, "cgact Active\n");
        retFun = bc95_cgact(0, 1);

        if (ATCMD_OK != retFun) {
            nb_delay(1000);
        }
    }while(ATCMD_OK != retFun);

    uint8_t msg[8];
    uint16_t idx;
    LOGDEBUG(BC95MODULE_DEBUG, "CGP addr\n"); 
    bc95_cgpaddr(0);
    bc95_socket("101.132.153.33", 5566);
    while(1){
        memset(sendBuffer, 0x00, sizeof(sendBuffer));
        bc95_sendMsg(0, "101.132.153.33", 5566, sendBuffer, 32);
        retVal = bc95_recvMsgPolling(0, msg, sizeof(msg), 10000);
        if (retVal > 0) {
            LOG("Receive Message[%d]:", retVal);
            for (idx = 0;idx < retVal;idx++){
                printf("[%02x]", msg[idx]);
            }
            printf("\n\n");
        }else {
            LOG("Receive Message Error\n");
        }
    }
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
*/

