#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* array;
    int status;// 0 is IDLE, 1 is BUSY
};


struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
int enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int rear(struct Queue* queue);
//int getStatus(struct Queue* queue);
//void setStatus(struct Queue* queue, int status);

struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->status = 0;
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity-1;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    return queue;
}

int isFull(struct Queue* queue)
{return (queue->size == queue->capacity);}

int isEmpty(struct Queue* queue)
{return (queue->size == 0);}

int enqueue(struct Queue* queue, int item)
{
    if(isFull(queue))
        return INT_MIN;
    queue->rear = (queue->rear+1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size+1;
    return 1;
}

int dequeue(struct Queue* queue){
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size-1;
    return item;
}

int front(struct Queue* queue)
{
    if(isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

int rear(struct Queue* queue)
{
    if(isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

// int getStatus(struct Queue* queue)
// {
//     return queue->status;
// }

// void setStatus(struct Queue* queue, int status)
// {
//     queue->status = status;
// }
