#ifndef LINKEDLIST
#define LINKEDLIST
#include<unistd.h>


typedef struct Node
{
    int id;
    struct Node * next;
}Node;


typedef struct LinkedList{
    Node * root;
    Node * last;
    int size;
}LinkedList;

void AddNode(LinkedList * list, int id);
LinkedList CreateLinkedList();
void DestroyLinkedList(LinkedList * list);


void AddNode(LinkedList * list, int id)
{
    Node * node = (Node*)malloc(sizeof(Node));
    node->id = id;
    node->next = NULL;
    if(list->root == NULL)
    {
        list->root = node;
        list->last = list->root;
    }
    else
    {
        list->last->next = node;
        list->last = list->last->next;
    }
    ++list->size;
}

LinkedList CreateLinkedList()
{
    LinkedList list;
    list.root = NULL;
    list.last = NULL;
    list.size = 0;
    return list;
}

void DestroyLinkedList(LinkedList * list)
{
    if(list->size > 0)
    {
        Node * one = list->root;
        Node * second = list->root;
        second = second->next;
        int i;
        for(i = 0; i < list->size; ++i)
        {
            free(one);
            if(second != NULL)
            {
                one = second;
                second = second->next;
            }
        }
    }
}
#endif