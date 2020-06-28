#ifndef GRAPH
#define GRAPH
#include<unistd.h>
#include"InputOutput.h"
#include"linkedList.h"
#include"queue.h"


typedef struct Graph{
    LinkedList * AdjList;
    int size;
    int edgeCount;
}Graph;

void CreateGraph(Graph * graph, int size);
int FillTheGraph(Graph * graph, char * filename);
void DestroyGraph(Graph * graph);
int GetMaxNodeID(char * filename);
int BreadthFirstSearch(Graph * graph, int startNode, int endNode, Queue * path);

void CreateGraph(Graph * graph, int size)
{
    graph->size = size;
    graph->edgeCount = 0;
    graph->AdjList = (LinkedList*)calloc(size, sizeof(LinkedList));
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
        if(graph->AdjList[i].size > 0)
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
        for(j = 0; j < graph.AdjList[i].size; ++j)
        {
            printf("%d, ", node->id);
            node = node->next;
        }
        printf("\n");
    }
}

int BreadthFirstSearch(Graph * graph, int startNode, int endNode, Queue * path)
{
    *path = CreateQueue();
    if(startNode == endNode)
    {
        Enqueue(path, startNode);
        Enqueue(path, endNode);
        return 1;
    }
    LinkedList reversePath = CreateLinkedList();
    Queue queue = CreateQueue();
    int pred[graph->size];
    int dist[graph->size];
    int visited[graph->size];
    int curNode,i,isTherePath = 0;
    for(i = 0; i < graph->size; ++i)
    {
        pred[i] = -1;
        visited[i] = 0;
        dist[i] = __INT_MAX__;
    }
    Enqueue(&queue, startNode);
    visited[startNode] = 1;
    dist[startNode] = 0;
    Node * node;
    while(!QueueIsEmpty(&queue))
    {
        curNode = Dequeue(&queue);
        // printf("curNode: %d\n", curNode);
        node = graph->AdjList[curNode].root;
        for(i = 0; i < graph->AdjList[curNode].size; i++)
        {
            if(!visited[node->id])
            {
                visited[node->id] = 1;
                dist[node->id] = dist[curNode] + 1;
                pred[node->id] = curNode;
                Enqueue(&queue, node->id);
            }
            node = node->next;
        }
        if(curNode == endNode)
        {
            isTherePath = 1;
            break;
        }
    }
    DestroyQueue(&queue);
    if(isTherePath)
    {
        int crawl = endNode;
        AddNode(&reversePath, crawl);
        while (pred[crawl] != -1) { 
            AddNode(&reversePath, pred[crawl]);
            crawl = pred[crawl]; 
        }
        int temp[reversePath.size];
        Node * node = reversePath.root;
        for(i = 0; i < reversePath.size; ++i)
        {
            temp[i] = node->id;
            node = node->next;
        }
        for(i = reversePath.size-1; i >= 0; --i)
        {
            Enqueue(path, temp[i]);
        }
        DestroyLinkedList(&reversePath);
        return 1;
    }
    DestroyLinkedList(&reversePath);
    return 0;
}
#endif