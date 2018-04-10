/*  
 * @file 	
 * @brief	
 * @details 
 * @author   purefarmer <xuzhedong@hzdusun.com> 
 * @date     2018.03.17 
 * @par Copyright (c):  RoomBanker Intelligent System CO.,LTD. 
 */
#include <unistd.h>
#include <sys/time.h>

#include "util.h"


/*  
 * @brief      using select timedelay
 * @param[out] None  
 * @param[in]  delay_ms -- delay time 
 * @return     None  
 * @see         
 * @note        
 */
void nb_delay(uint32_t delay_ms)
{
  struct timeval tv;

  tv.tv_sec = delay_ms / 1000;
  tv.tv_usec = (delay_ms % 1000) * 1000;
  select(0, NULL, NULL, NULL, &tv);
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
static void getNext(const uint8_t* p, uint32_t pLen, int next[])    
{    
	next[0] = -1;    
	int k = -1;    
	int q = 0;    
	while (q < pLen - 1){    
		//p[k]表示前缀，p[j]表示后缀    
		if (k == -1 || p[q] == p[k]){    
			++k;    
			++q;    
			next[q] = k;    
		}else {
			k = next[k];    
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
int kmpCmp(const uint8_t t[], uint32_t tLen, const uint8_t p[], uint32_t pLen, int next[])
{
	/* TODO  */
	return -1;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int bfCmp(const uint8_t t[], uint32_t tLen, const uint8_t p[], uint32_t pLen)
{
	uint32_t ti, pi;
	
	if (pLen > tLen)
	{
		return -1;
	}
	for(ti = 0;ti < tLen -pLen;ti++)
	{
		for (pi = 0;pi < pLen;pi++)	
		{
			if (t[ti + pi] != p[pi])	
			{
				break;
			}
		}
		if (pi == pLen)
		{
			return ti;
		}
	}
	return -1;
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
int strhex2hex_byte(char *str, int size, unsigned char *hex)
{
	if (size != 2) {
		return -1;
	}

	int idx;
	unsigned char halfHex[2];

	for(idx = 0;idx < size;idx++)
	{
		if ('0' <= str[idx] && str[idx] <='9')
		{
			halfHex[idx] = str[idx] - '0';
		}else if ('a' <= str[idx] && str[idx] <= 'f'){
			halfHex[idx] = str[idx] - 'a' + 10;
		}else if ('A' <= str[idx] && str[idx] <='F') {
			halfHex[idx] = str[idx] - 'A' + 10;
		}
		else{
			*hex = 0;
			return -1;
		}
	}
	*hex = (halfHex[0] << 4) + halfHex[1];
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
int strdec2dec_uint32(const char *str, unsigned int size, unsigned int *value)
{
    if (size < 1 || size > 32) {
        return -1;
    }
    
    int idx;
    unsigned int dec[32], decValue = 0;

    for (idx = 0; idx < size;idx++)
    {
        if ('0' <= str[idx] && str[idx] <= '9')
        {
           decValue = decValue * 10;
           decValue += str[idx] - '0';
        }else{
            return -2;
        }
    }
    *value = decValue;
    
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
int mysscan_uint(const char str[], unsigned int *valuePtr)
{
    const char *ptr = str;
    while(*ptr != '\0') 
    {
        if (*ptr == '\r' || *ptr == '\n' || *ptr == ' ')
        {
            return strdec2dec_uint32(str, ptr - str, valuePtr);
        }
        ptr++;
    }
    return -1;
}


/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
 int createThread(int32_t *threadId, ThreadFun_t threadFun, void *arg)
 {
	pthread_t pthreadID;
	int retFun;
	
	return pthread_create(&pthreadID, NULL, threadFun, arg);
 }

/*  
 * @brief      create Mutex 
 * @param[out] mutex -- mutex pointer  
 * @param[in]  None 
 * @return     0 -- success;other -- failed  
 * @see         
 * @note        
 */
int mutexInit(Mutex_t *mutex)
{
   return pthread_mutex_init(mutex, NULL);
}

/*  
 * @brief      mutex lock 
 * @param[out] None   
 * @param[in]  mutex -- mutex pointer 
 * @return     0 -- OK;other --failed  
 * @see         
 * @note        
 */
int mutexLock(Mutex_t *mutex)
{
    return pthread_mutex_lock(mutex);
}


/*  
 * @brief      Unlock Mutex 
 * @param[out] None  
 * @param[in]  mutex -- mutex pointer 
 * @return     0 -- ok;other -- failed  
 * @see         
 * @note        
 */
int mutexUnlock(Mutex_t *mutex)
{
    return pthread_mutex_unlock(mutex); 
}

/*  
 * @brief        
 * @param[out]   
 * @param[in]   
 * @return       
 * @see         
 * @note        
 */
