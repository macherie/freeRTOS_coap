/*  
 * @file 	log.h
 * @brief	log 日志
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018-1-28  
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


#define LOG(fmt,arg...)	        	    do{priPrintf("%s() #%d "fmt, __FUNCTION__, __LINE__,##arg);}while(0)
#define LOGERROR(fmt,arg...)    	    do{priPrintf("%s() #%d "fmt, __FUNCTION__, __LINE__,##arg);}while(0)
    
#define LOGDEBUG(permit,fmt, arg...)    do{if(permit){LOG(fmt,##arg);}}while(0)
	

void priPrintfInit(void);

int priPrintf(const char *format,...);




#ifdef __cplusplus 
} 
#endif

#endif /* log.h */
