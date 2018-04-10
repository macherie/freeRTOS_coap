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

#include <stdio.h>

#ifdef __cplusplus 
extern "C" 
{ 
#endif 


#include <stdio.h>

#define LOG(fmt,arg...)	        	    do{printf("[%s] %s() #%d "fmt,__FILE__, __FUNCTION__, __LINE__,##arg);}while(0)
#define LOGERROR(fmt,arg...)    	    do{fprintf(stderr,"[%s] %s() #%d "fmt,__FILE__, __FUNCTION__, __LINE__,##arg);}while(0)
    
#define LOGDEBUG(permit,fmt, arg...)    do{if(permit){LOG(fmt,##arg);}}while(0)


#ifdef __cplusplus 
} 
#endif

#endif /* log.h */
