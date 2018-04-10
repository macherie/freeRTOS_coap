/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <string.h>

#include "uartComm.h"
#include "bc95Module.h"
#include "nbiotCtrl.h"
#include "util.h"
#include "log.h"


typedef struct
{
    int socket;
    char ip[64];
    uint16_t port;
    uint8_t serialSetup;
    uint8_t netBuild;
    NbCtrlStatus_t state;
}NbCtrlCtx_t;


static NbCtrlCtx_t gCtx;

void nbIoTSetState(NbCtrlStatus_t expect);

/*  
 * @brief      
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static int moduleIsReg(uint8_t reptTimes)
{
    CMDStatus_t retFun;

    reptTimes=reptTimes?reptTimes:1;

    do{
        LOGDEBUG(1,"Check BC95 Is Registed\n");
        retFun = bc95_isRegist();
        if (ATCMD_OK != retFun) {
            nb_delay(2000);
            if (0 == --reptTimes) {
                LOG("Registed Time Out");
                return -1;
            }
        }
    }while(ATCMD_OK != retFun);

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
 int nbIoTRestart(void)
 {
    CMDStatus_t retFun;
    int cnt = 0, retVal;

    //nbIoTSetState(NBCTRL_NOTINITD);

    nbIoTSetState(NBCTRL_CONNECTING);
    do{
        LOGDEBUG(1,"Reset BC95 Module\n");
        retFun = bc95_rest();
        if (ATCMD_OK != retFun) {
            nb_delay(2000);
            if (cnt > 7) {
                nbIoTSetState(NBCTRL_NOCONNECTED);
                LOG("Reset Reapte Out\n");
                return -1;
            }
        }
    }while(ATCMD_OK != retFun);

    bc95_noEcho();

    retVal = moduleIsReg(20);
    if (retVal != 0) {
        nbIoTSetState(NBCTRL_NOCONNECTED);
        LOG("Regist Error\n");
        return -1;
    }

    LOGDEBUG(1, "cgdcont Set\n");
    retFun = bc95_cgdcontSet(1, "IP", DEFAULT_APN);
    if (ATCMD_OK != retFun) {
        nbIoTSetState(NBCTRL_NOCONNECTED);
        LOG("Set Cgdcont Error\n");
        return -1;
    }
    
    nb_delay(500);
    LOGDEBUG(1, "cgact Active\n");
    retFun = bc95_cgact(0, 1);
    if (ATCMD_OK != retFun) {
        nbIoTSetState(NBCTRL_NOCONNECTED);
        LOG("Cgact Active Error\n");
        return -1;
    }

    LOGDEBUG(1, "CGP addr\n"); 
    bc95_cgpaddr(0);

    /* Create Socket */
    LOGDEBUG(1, "Create Socket\n"); 
    retVal = bc95_socket(NULL, 5566);
    if (retVal < 0) {
        nbIoTSetState(NBCTRL_NOCONNECTED);
        LOG("Create Socket Error\n");
        return -1;
    }
    gCtx.socket = retVal;
    gCtx.netBuild = 1;
    nbIoTSetState(NBCTRL_CONNECTED);
    return 0;
 }

/*  
 * @brief      NBIoT Module Initiazlie
 * @param[out] None
 * @param[in]  ip -- peer ip; port -- peer port
 * @return     <0 -- error;>=0 -- ok
 * @see         
 * @note        
 */
int nbIoTInit(const char ip[], uint16_t port)
{
    CMDStatus_t retFun;

    /* initialize uart */
    retFun = uartComm_init();
    if (retFun != 0) {
        LOG("Uart Initialize Error\n");
        return -1;
    }
    gCtx.serialSetup = 1;

    /* copy IP and Port */
    if (strlen(ip) == 0) {
        LOG("IP Empty\n");
        return -1;
    }
    strcpy(gCtx.ip, ip);
    gCtx.port = port;
    nbIoTSetState(NBCTRL_CLOSED);

    return 0;
}


/*  
 * @brief      send message by NBIoT module
 * @param[out] None
 * @param[in]  data -- data buffer; dataLen -- data Length
 * @return       
 * @see         
 * @note        
 */
int nbIoTSendMsg(const uint8_t data[], uint16_t dataLen)
{
    int retVal;

    if (NBCTRL_CONNECTED != nbIoTGetState()) {
        LOG("NBIoT Module Not Connected\n");
        return -1;
    }
    nbIoTSetState(NBCTRL_SENDING);
    retVal = bc95_sendMsg(gCtx.socket, gCtx.ip, gCtx.port, data, dataLen);
    if (retVal < 1) {
        LOG("Send Message Error\n");

    }
    nbIoTSetState(NBCTRL_CONNECTED);
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
int nbIoTRecvMsg(uint8_t data[], uint16_t dataSize, uint32_t timeOut)
{
    int retVal; 

    if (NBCTRL_CONNECTED != nbIoTGetState()) {
        LOG("NBIoT Module Not Connected\n");
        return -1;
    }
    nbIoTSetState(NBCTRL_RECVING);
    retVal = bc95_recvMsgPolling(gCtx.socket, data, dataSize, timeOut);
    if (retVal < 1) {
        LOG("NBIoT Module Receiving Error:%d\n", retVal);
        retVal = 0;
    }
    nbIoTSetState(NBCTRL_CONNECTED);
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
NbCtrlStatus_t nbIoTGetState(void)
{
    return gCtx.state;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
void nbIoTSetState(NbCtrlStatus_t expect)
{
    gCtx.state = expect;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int nbIoTSetPeer(const char ip[], uint16_t port)
{
    /* copy IP and Port */
    if (strlen(ip) == 0) {
        LOG("IP Empty\n");
        return -1;
    }
    strcpy(gCtx.ip, ip);
    gCtx.port = port;
    return 0;
}
