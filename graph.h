#ifndef GRAPH
#define GRAPH
#include<unistd.h>
#include"InputOutput.h"

typedef struct Graph{
    int ** AdjList;
    int size;
}Graph;

void CreateGraph(Graph * graph, int size);
int FillTheGraph(Graph * graph, char * filename);
void DestroyGraph(Graph * graph);
int GetMaxNodeID(char * filename);

void CreateGraph(Graph * graph, int size)
{
    graph->size = size;
    graph->AdjList = (int**)calloc(sizeof(int*), size);
    int i;
    for(i = 0; i < size; ++i)
    {
        graph->AdjList[i] = (int*)calloc(sizeof(int), size);
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
            graph->AdjList[first][second] = 1;
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
        free(graph->AdjList[i]);
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
#endif