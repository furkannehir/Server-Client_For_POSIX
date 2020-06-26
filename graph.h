#ifndef GRAPH
#define GRAPH
#include<unistd.h>
#include"InputOutput.h"

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

typedef struct Graph{
    LinkedList * AdjList;
    int size;
    int edgeCount;
}Graph;

void AddNode(LinkedList * list, int id);
LinkedList CreateLinkedList();
void DestroyLinkedList(LinkedList * list);
void CreateGraph(Graph * graph, int size);
int FillTheGraph(Graph * graph, char * filename);
void DestroyGraph(Graph * graph);
int GetMaxNodeID(char * filename);

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
    Node * one = list->root;
    Node * second = list->root;
    second = second->next;
    int i;
    for(i = 0; i < list->size; ++i)
    {
        free(one);
        one = second;
        second = second->next;
    }
}

void CreateGraph(Graph * graph, int size)
{
    graph->size = size;
    graph->edgeCount = 0;
    graph->AdjList = (LinkedList*)calloc(sizeof(LinkedList), size);
    int i;
    for(i = 0; i < size; ++i)
    {
        graph->AdjList[i] = CreateLinkedList();
    }
}

int FillTheGraph(Graph * graph, char * filename)
{
    int fd = OpenFile(filename, READ);
    if(fd == -1)
        return -1;
    int first, second;
    char buf[1000];
    while(ReadOneLine(filename, fd, buf))
    {
        if(buf[0] != '#')
        {
            sscanf(buf, "%d  %d\n", &first, &second);
            if(first > graph->size || first < 0)
                return -1;
            if(second > graph->size || second < 0)
                return -1;
            AddNode(&graph->AdjList[first], second);
            ++graph->edgeCount;
        }
    }
    CloseFile(filename, fd);
    return 1;
}

void DestroyGraph(Graph * graph)
{
    int i;
    for(i = 0; i < graph->size; ++i)
    {
        DestroyLinkedList(&graph->AdjList[i]);
    }
    free(graph->AdjList);
}

int GetMaxNodeID(char * filename)
{
    char buf[1000];
    int fd = OpenFile(filename, READ);
    if(fd == -1)
        return -1;
    int first, second, max = 0;
    while(ReadOneLine(filename, fd, buf))
    {
        if(buf[0] != '#')
        {
            sscanf(buf, "%d  %d\n", &first, &second);
            if(first > max)
                max = first;
            if(second > max)
                max = second;
        }
    }
    CloseFile(filename, fd);
    return max;
}

void PrintGraph(Graph graph)
{
    int i,j;
    for(i = 0; i < graph.size; ++i)
    {
        Node * node = graph.AdjList[i].root;
        printf("%d: ", i);
        // printf("size: %d", graph.AdjList[i].size);
        for(j = 0; j < graph.AdjList[i].size; ++j)
        {
            printf("%d, ", node->id);
            node = node->next;
        }
        printf("\n");
    }
}
#endif