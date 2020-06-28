#ifndef QUEUE
#define QUEUE
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include"linkedList.h"

typedef struct Queue{
    LinkedList list;
}Queue;

Queue CreateQueue();
void Enqueue(Queue * queue, int value);
int Dequeue(Queue * queue);
int QueueIsEmpty(Queue * queue);
void DestroyQueue(Queue * queue);
void PrintQueue(Queue * queue);
void QueueString(Queue queue, char * str);



Queue CreateQueue()
{
    Queue queue;
    queue.list = CreateLinkedList();
    return queue;
}

void Enqueue(Queue * queue, int value)
{
    AddNode(&queue->list, value);
}

int Dequeue(Queue * queue)
{
    Node * node = queue->list.root;
    if(QueueIsEmpty(queue))
        return -1;
    int value = queue->list.root->id;
    queue->list.root = queue->list.root->next;
    --queue->list.size;
    free(node);
    return value;
}

int QueueIsEmpty(Queue * queue)
{
    if(queue->list.size == 0)
        return 1;
    else
        return 0;
}

void DestroyQueue(Queue * queue)
{
    DestroyLinkedList(&queue->list);
}

void PrintQueue(Queue * queue)
{
    if(QueueIsEmpty(queue))
        return;
    Node * node = queue->list.root;
    printf("%d", node->id);
    node = node->next;
    int i;
    for(i = 1; i < queue->list.size; ++i)
    {
        printf("->%d", node->id);
        node = node->next;
    }
    printf("\n");
}


void QueueString(Queue queue, char * str)
{
    if(QueueIsEmpty(&queue))
    {
        strcpy(str, "There's no path!");
    }
    else
    {
        char buffer[10];
        Node * node = queue.list.root;
        sprintf(buffer, "%d", node->id);
        strcat(str, buffer);
        node = node->next;
        int i;
        for(i = 1; i < queue.list.size; ++i)
        {
            sprintf(buffer, "->%d", node->id);
            strcat(str, buffer);
            node = node->next;
        }
    }
}
#endif