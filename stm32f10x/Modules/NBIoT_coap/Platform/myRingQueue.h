

#ifndef MYRINGQUEUE_H
#define MYRINGQUEUE_H


#include <stdint.h>

typedef struct _ringQueue
{
	uint8_t *buffer;
	uint32_t size;
	uint32_t front;
	uint32_t rear;

}ringQueue_t;


void initQueue(ringQueue_t *thisQueue, uint8_t *buffer, uint32_t bufferSize);

int enQueue(ringQueue_t *thisQueue, uint8_t data);

int popQueue(ringQueue_t *thisQueue, uint8_t *data);

int lenQueue(const ringQueue_t *thisQueue);




#endif /* myQueue.h */