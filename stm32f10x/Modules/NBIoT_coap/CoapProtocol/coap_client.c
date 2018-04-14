/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <stdio.h>
#include <string.h>

#include "coap_client.h"
#include "er-coap-13-transactions.h"
#include "uri.h"
#include "log.h"


#define DEBUG 1


typedef struct
{
	uint16_t port;
	char host[32];
	char path[64];
	
	coap_packet_t rsp[1];
    coap_packet_t req[1];
	coap_transaction_t trans[1];
}CoapClientCtx_t;


static CoapClientCtx_t gCtx;
static uint8_t gUdpRecvBuffer[256];


/*  
 * @brief       URI Parse
 * @param[out]  ip -- ip buffer;
 * @param[in]   uriStr -- uri string;ipSize -- ip buffer size
 * @return      >0 -- port;other -- failed
 * @see         
 * @note        
 */
 int coapUriParseGetIp(const char uriStr[], char ip[], uint16_t ipSize)
{
	coap_uri_t *uri = NULL;
    int retVal = 0, retFun;

	if (NULL == uriStr) {
		LOG("No URI String\n");
		return -1;
	}
	uri = coap_new_uri((unsigned char*)uriStr, strlen(uriStr));   
    if (uri == NULL) {
        LOG("coap_new_uri(): URI failed\n");
		return -2;
    }
    snprintf(ip, ipSize, "%s", uri->host.s);
    ip[uri->host.length] = '\0';
	retVal = uri->port;
    sprintf(gCtx.path, "%s", uri->path.s);
    memcpy(gCtx.host, uri->host.s, uri->host.length);
    gCtx.host[uri->host.length] = '\0';
	gCtx.port = uri->port;

    LOGDEBUG(DEBUG, "URI Path: %s\n", uri->path.s);
    LOGDEBUG(DEBUG, "URI Host: %s:%d\n", ip, retVal);

    free(uri);

	return retVal;
}

/*  
 * @brief       coap Message Serialize 
 * @param[out]  msgBuffer -- serialize buffer pointer
 * @param[in]   paylaod -- payload data; payloadSize -- payload size; 
 *              coapType -- coap message type; method -- coap method;
 *              msgSize -- serialize buffer size
 * @return      0 -- error; >0 send size
 * @see         
 * @note        
 */
 int coapClientSerial(uint8_t payload[], uint16_t payloadSize, int coapType, int method, uint8_t msgBuffer[], uint16_t msgSize)
{
	if (payloadSize > COAP_MAX_HEADER_SIZE)
	{
		LOGERROR("Payload Size too Big\n");
		return 0;
	}

	int retVal;
    // init a CoAP message and set message header
    coap_init_message(gCtx.req, coapType, method, coap_get_mid());
    coap_set_header_uri_path(gCtx.req, gCtx.path);
    //coap_set_header_uri_host(gCtx.req, gCtx.host);

 	LOG("Payload Size: %d\n", payloadSize);

    // CoAP payload
    coap_set_payload(gCtx.req, payload, payloadSize);

    // Build CoAP header and Options
   	gCtx.trans->packet_len = coap_serialize_message(gCtx.req, gCtx.trans->packet);

	LOG("Requested MID %u\n", gCtx.req->mid);
   	LOG("Header dump: [0x%02X %02X %02X %02X]. Size: %d\n",
                gCtx.req->buffer[0],
                gCtx.req->buffer[1],
                gCtx.req->buffer[2],
                gCtx.req->buffer[3],
                gCtx.trans->packet_len);
    if (msgSize < gCtx.trans->packet_len) {
        LOG("Message Buffer too Small.\n");
        return 0;
    }
    memcpy(msgBuffer, gCtx.trans->packet, gCtx.trans->packet_len); 
    retVal = gCtx.trans->packet_len;
    return retVal;
}

/*  
 * @brief      parse message
 * @param[out] payload -- payload pointer  
 * @param[in]  payloadSize -- payload size; msgBuffer -- message buffer pointer;
 *             msgSize -- message buffer size
 * @return     payload size 
 * @see         
 * @note        
 */
int coapClientParse(uint8_t payload[], uint16_t payloadSize, uint8_t msgBuffer[], uint16_t msgSize)
{
    
    int retFun = 0;
    coap_status_t erbium_status_code;
    const uint8_t *payloadPtr = NULL;
    //uint16_t copyLen;

    /* Parse Message */
    erbium_status_code = coap_parse_message(gCtx.rsp, msgBuffer, msgSize);
    if (NO_ERROR == erbium_status_code)
    {
	    if(gCtx.rsp->type == COAP_TYPE_CON && gCtx.rsp->code == 0) {
		    LOG("Received Ping\n");
	    } else if(gCtx.rsp->type == COAP_TYPE_ACK) {
		    LOG("Received ACK\n");
		    retFun = coap_get_payload(gCtx.rsp, &payloadPtr);
	    } else if (gCtx.rsp->type == COAP_TYPE_CON){
		    LOG("Received CON\n");
            retFun = coap_get_payload(gCtx.rsp, &payloadPtr);
	    } else if(gCtx.rsp->type == COAP_TYPE_RST) {
		    LOG("Received RST\n");
	    }
    }else{
        LOG("Message Parse Error\n");
    }
    if (NULL == payloadPtr){
        LOGERROR("No Payload Data\n");
        return 0;
    }
    if (retFun > payloadSize)
    {
        LOGERROR("Payload Size too Small\n");
        return 0;
    }
    memcpy(payload, payloadPtr, retFun);
    return retFun;
}

