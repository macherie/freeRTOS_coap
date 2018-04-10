

#include "myRingQueue.h"

/*
    queue initialize
*/
void initQueue(ringQueue_t *thisQueue, uint8_t *buffer, uint32_t bufferSize)
{
    thisQueue->buffer = buffer;
    thisQueue->size = bufferSize;
    thisQueue->front = thisQueue->rear = 0;
}

/*
    push Queue
*/
int enQueue(ringQueue_t *thisQueue, uint8_t data)
{
    uint32_t *rearPtr = 0, front, nextRear;

    front = thisQueue->front;
    rearPtr = &(thisQueue->rear);
    nextRear = (*rearPtr + 1) % (thisQueue->size);
    if (front == nextRear)
    {
        return 0;
    }

    thisQueue->buffer[(*rearPtr)++] = data;
    *rearPtr = (*rearPtr) % thisQueue->size;

    return 1;
}

/*
    pop queue
*/
int popQueue(ringQueue_t *thisQueue, uint8_t *data)
{
    uint32_t *frontPtr = 0;

    if (thisQueue->front == thisQueue->rear)
    {
        return 0;
    }
    frontPtr = &(thisQueue->front);
    *data = thisQueue->buffer[(*frontPtr)++];
    *frontPtr = *frontPtr % thisQueue->size;
    return 1; 
}

/*
    length of queue
*/
int lenQueue(const ringQueue_t *thisQueue)
{
    int length;

    length = (thisQueue->rear + thisQueue->size - thisQueue->front) % thisQueue->size;

    return length;
}

