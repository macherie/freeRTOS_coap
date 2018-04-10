/*  
 * @file 	
 * @brief	
 * @details 
 * @author   Purefarmer <xuzhedong@hzdusun.com> 
 * @date     
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

#include <stdint.h>
#include <pthread.h>


typedef void *(*ThreadFun_t)(void *arg);
typedef pthread_mutex_t Mutex_t ;


void nb_delay(uint32_t delay_ms);

int bfCmp(const  uint8_t *,uint32_t,const uint8_t *,uint32_t);

int strhex2hex_byte(char *str, int size, unsigned char *hex);

int strdec2dec_uint32(const char *str, unsigned int size, unsigned int *value);

int mysscan_uint(const char str[], uint32_t *valuePtr);

int createThread(int32_t *threadId, ThreadFun_t threadFun, void *arg);

int mutexInit(Mutex_t *mutex);

int mutexLock(Mutex_t *mutex);

int mutexUnlock(Mutex_t *mutex);


#ifdef __cplusplus 
} 
#endif

#endif /* util.h */
