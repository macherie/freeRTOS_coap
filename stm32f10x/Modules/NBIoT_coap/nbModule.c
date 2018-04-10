/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <stdint.h>
#include <string.h>

#include "log.h"

#include "util.h"
#include "bc95Module.h"
#include "nbiotCtrl.h"
#include "nbProtclExam.h"
#include "nbModule.h"


#define MAXERRORCOUNTER                 (3)
#define SendRecvErrCounter(retFunc)     do{if (retFunc == -2 || retFunc == -3){\
                                            if (++gCtx.errCnter >= MAXERRORCOUNTER)\
                                             {nbIoTSetState(NBCTRL_NOCONNECTED);\
                                                LOG("NBIoT Module Disconnected\n");gCtx.errCnter=0;}}\
                                            else{gCtx.errCnter=0;};\
                                        }while(0)


/* all ctroller struct */
typedef struct
{
    uint8_t isRestart;          /* Restart 1 -- restart; */
    uint8_t operate;            /* Oprate 1 -- have data then oprate;0 -- no data no operate */
    uint8_t errCnter;
    int32_t threadID;           /* thread ID */

    CmdHandlerFunc_t    cmdHandler;
    CmdErrHandlerFunc_t errHandler;
    Mutex_t mutex;
}NbCtx_t;

/* cmd struct */
typedef struct 
{
    uint8_t cmd;
    uint8_t txrxbuffer[NBLOCK_PROTCL_PAYLOAD_SIZE];
    uint16_t dataLen;
}CmdMsg_t;


static NbCtx_t gCtx;
static CmdMsg_t gMsg;
static NblockFrame_t gFrame;

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
 static void cmdHandler(void)
 {
    uint8_t cmd = gMsg.cmd;
    int retFunc;

    retFunc = cmdAttachedFuncExe(cmd, gMsg.txrxbuffer, &gFrame);
    SendRecvErrCounter(retFunc);
    if (0 == retFunc){
        gCtx.cmdHandler(gFrame.cmd, gFrame.data, gFrame.dataLen);
    }else{
        LOG("CMD Exe Error\n");
        gCtx.errHandler(cmd);
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
static void *nbProcess(void *arg)
{
    int state;

    for (;;) {
        state = nbIoTGetState();
        switch(state){
            case NBCTRL_CLOSED:
	                nbIoTRestart();                /* restart NBIoT Module */
                    break;
            case NBCTRL_CONNECTED: 
                    if (1 == gCtx.operate){
                        cmdHandler();              /* cmd Message Handler */
                        mutexLock(&gCtx.mutex);
                        gCtx.operate = 0;
                        mutexUnlock(&gCtx.mutex);
                    }
                    break;
            case NBCTRL_NOCONNECTED:
                    break;
            default:
                    break;
        }

        if (1 == gCtx.isRestart) {
            mutexLock(&gCtx.mutex);
            gCtx.isRestart = 0;
            mutexUnlock(&gCtx.mutex);
            nbIoTSetState(NBCTRL_CLOSED);  
        }
        nb_delay(10); 
    }
    return NULL;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nbInit(CmdHandlerFunc_t handler, CmdErrHandlerFunc_t errHandler)
{
    int retFun;
	uint8_t mac[8];

    if (NULL == handler || NULL == errHandler){
        LOG("No Callback Function\n");
        return -1;
    }
    gCtx.cmdHandler = handler;
    gCtx.errHandler = errHandler;

	/* NB module Initialize */
    retFun = nbIoTInit(URI_IP, 5683);
	if (retFun < 0){
		LOG("NB Module Initialize Error\n");
		return -2;
	}

	/* Get Module Mac */
	retFun = bc95_serialNum(mac, sizeof(mac));
	if (retFun < 1){
		LOG("Get Module mac Error\n");
		return -3;
	}

	/* Protocol Initialize */
	retFun = nblockProtclInit(mac, retFun);
	if (retFun < 0){
		LOG("NB Protocol Initialize Error\n");
		return -4;
	}
    /* create mutex */
    retFun = mutexInit(&gCtx.mutex);
    if (0 != retFun){
        LOG("Create Mutex Error\n");
        return -5;
    }

    /* create thead */
    retFun = createThread(&gCtx.threadID, nbProcess, NULL);
    if (0 != retFun) {
        LOG("Create Thread Error.\n");
        return -6;
    }
	return 0;
}

/*  
 * @brief      get NBIoT Module Status 
 * @param[out] None  
 * @param[in]  None 
 * @return     NBIoT Status  
 * @see         
 * @note        
 */
int nbStatus(void)
{
    return nbIoTGetState();
}

/*  
 * @brief      user send message to server
 * @param[out] None
 * @param[in]  cmd -- frame cmd; data -- frame data pointer;
 *             dataLen -- frame data length;
 * @return       
 * @see         
 * @note        
 */
 int nbMsgSend(uint8_t cmd, const uint8_t data[], uint16_t dataLen)
 {
    if (NBCTRL_CONNECTED != nbStatus()){
        LOG("NBIoT Module Status Error, not Connected.\n");
        return -1;
    }
    if (dataLen > NBLOCK_PROTCL_PAYLOAD_SIZE){
        LOG("Data too Big.\n");
        return -2;
    }
    if (1 == gCtx.operate){
        LOG("NBIoT Module is Operating.\n");
        return -3;
    }

    /* fill data */
    if (dataLen > 1){
        memcpy(gMsg.txrxbuffer, data, dataLen);
    }else if (1 == dataLen){
        gMsg.txrxbuffer[0] = data[0];
    }
    gMsg.dataLen = dataLen;
    gMsg.cmd = cmd;

    mutexLock(&gCtx.mutex);
    gCtx.operate = 1;
    mutexUnlock(&gCtx.mutex);
    return dataLen;
}

/*  
 * @brief      restart NBIoT Module
 * @param[out] None  
 * @param[in]  None 
 * @return     None  
 * @see         
 * @note        
 */
void nbRestart(void)
{
    mutexLock(&gCtx.mutex);
    gCtx.isRestart = 1;
    mutexUnlock(&gCtx.mutex);
}



