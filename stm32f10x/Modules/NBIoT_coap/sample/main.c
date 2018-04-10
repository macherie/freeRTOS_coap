
#include "log.h"
#include "util.h"
#include "nbiotCtrl.h"
#include "nbModule.h"
#include "nblockProtcl.h"
#include "nbProtclExam.h"

#include <stdio.h>


void cmdHandler(uint8_t cmd, const uint8_t data[], uint16_t dataLen);    /* 命令处理函数 */
void cmdErrHandler(uint8_t cmd);

void test_joinNet(void);
void test_ping(void);
void test_netservice(void);
void test_rptAttr(void);
void test_otaData(void);
void test_rspOpenlock(void);
void test_rspOtaStart(void);
void test_rspAddAttr(void);
void test_rspDelAttr(void);
void test_rspModAttr(void);
void test_rspQryAttr(void);

int main (void)
{
	int retVal;
    NblockFrame_t frame;

	retVal = nbInit(cmdHandler, cmdErrHandler);
	if (retVal < 0)
	{
		printf("NBIoT Initialize Error\n");
		return 1;
	}
    for (;;) {
        if (nbStatus() == NBCTRL_CONNECTED){
            //test_joinNet();
            //test_ping();
            //test_netservice();
            //test_rptAttr();
            //test_otaData();
            //test_rspOpenlock();
            //test_rspOtaStart();
            //test_rspAddAttr();
            //test_rspDelAttr();
            //test_rspModAttr();
            test_rspQryAttr();
        }else if (nbStatus() == NBCTRL_NOCONNECTED){
            nbRestart();
        }
        
        nb_delay(1000);
    }
    
	return 0;
}

void test_joinNet(void)
{
    nbMsgSend(NBLOCK_UPCMD_PERMIT, NULL, 0);
}

void test_ping(void)
{
    Ping_t ping;

    ping.time= 0x0001;
    ping.battery = 90;
    nbMsgSend(NBLOCK_PING, (uint8_t *)&ping, sizeof(ping));
}

void test_netservice(void)
{
    General_t netservice;

    netservice.data = 0x01;
    nbMsgSend(NBLOCK_UPCMD_NETSERVICE, (uint8_t *)&netservice, sizeof(netservice));
}

void test_rptAttr(void)
{
    RptAttrReq_t req;

    req.rpt_reason = 0x01;
    req.attrType = 0x03;
    req.attrIdx = 0x09;
    req.attrPos = 0x10;
    req.len = 0x03;
    req.data[0] = 0x00;
    req.data[1] = 0x02;
    req.data[2] = 0x04;
    nbMsgSend(NBLOCK_UPCMD_RPTATTR, (uint8_t *)&req, sizeof(req));
}

void test_otaData(void)
{
    OtaDataReq_t req;
    req.target = 0x01;
    req.sw_ver = 0x0002;
    req.pos = 0xdeedbeef;
    req.len = 0xbeefdead;
    nbMsgSend(NBLOCK_UPCMD_OTADATA, (uint8_t *)&req, sizeof(req));
}

void test_rspOpenlock(void)
{
    uint8_t ret;        // 0x01 -- open door OK,0x00 -- open failed
    uint8_t reason;
    LockOpenRsp_t rsp;

    rsp.ret = 0x01;
    rsp.reason = 0x00;
    nbMsgSend(NBLOCK_DOWNCMD_OPENLOCK, (uint8_t *)&rsp, sizeof(rsp));
}

void test_rspOtaStart(void)
{
    OtaStartRsp_t rsp;
    rsp.ret = 0x01;
    rsp.target = 0;
    nbMsgSend(NBLOCK_DOWNCMD_OTASTART, (uint8_t *)&rsp, sizeof(rsp));
}

void test_rspAddAttr(void)
{
    AddDelAttrRsp_t rsp;

    rsp.ret = 0x01;
    rsp.attrType = 0xab;
    rsp.attrIdx = 0xcf;
    nbMsgSend(NBLOCK_DOWNCMD_ADDATTR, (uint8_t *)&rsp, sizeof(rsp));
}

void test_rspDelAttr(void)
{
    AddDelAttrRsp_t rsp;
    rsp.ret = 0x01;
    rsp.attrType = 0xab;
    rsp.attrIdx = 0xcf;
    nbMsgSend(NBLOCK_DOWNCMD_DELATTR, (uint8_t *)&rsp, sizeof(rsp));
}

void test_rspModAttr(void)
{
    ModAttrRsp_t rsp;
    rsp.ret = 0x02;
    rsp.attrType = 0xcffc;
    rsp.attrIdx = 0xabba;
    rsp.pos = 0xbeefdead;
    rsp.len = 0xdeadbeef;

    nbMsgSend(NBLOCK_DOWNCMD_MODATTR, (uint8_t *)&rsp, sizeof(rsp));
}

void test_rspQryAttr(void)
{

    QryAttrRsp_t rsp;
    rsp.ret = 0x04;
    rsp.attrType = 0xabcd;
    rsp.attrIdx = 0xdeadbeef;
    rsp.attrPos = 0xbeefdead;
    rsp.len = 0x01;
    rsp.data[0] = 0xfe;
    nbMsgSend(NBLOCK_DOWNCMD_QRYATTR, (uint8_t *)&rsp, sizeof(rsp));
}

void cmdHandler(uint8_t cmd, const uint8_t data[], uint16_t dataLen)
{
    LOG("CMD Handler OK.\n");
}

void cmdErrHandler(uint8_t cmd)
{
    LOG("CMD Handler Error.\n");
}
