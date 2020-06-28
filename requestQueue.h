#ifndef REQUESTQUEUE
#define REQUESTQUEUE
#include<unistd.h>
#include<stdlib.h>


typedef struct requestNode{
    int source;
    int destination;
    int fileDesc;
    struct requestNode * next;    
}requestNode;

typedef struct requestQueue{
    requestNode * root;
    requestNode * last;
    int size;
}requestQueue;

requestQueue CreateRequestQueue();
void EnqueueRequest(requestQueue * req, requestNode * node);
requestNode DequeueRequest(requestQueue * req);
int RequestIsEmpty(requestQueue req);
requestNode CreateRequestNode(int source, int dest, int fileDesc);


requestQueue CreateRequestQueue()
{
    requestQueue req;
    req.root = NULL;
    req.last = NULL;
    req.size = 0;
    return req;
}

void EnqueueRequest(requestQueue * req, requestNode * node)
{
    if(RequestIsEmpty(*req))
    {
        req->root = node;
        req->last = node;
    }
    else
    {
        req->last->next = node;
        req->last = req->last->next;
    }
    ++req->size;
}

requestNode DequeueRequest(requestQueue * req)
{
    if(RequestIsEmpty(*req))
        return CreateRequestNode(-1, -1, -1);
    requestNode node;
    node = *(req->root);
    requestNode* rootAddr = req->root;
    req->root = req->root->next;
    free(rootAddr);
    return node;
}

int RequestIsEmpty(requestQueue req)
{
    if(req.size == 0)
        return 1;
    return 0;
}

requestNode CreateRequestNode(int source, int dest, int fileDesc)
{
    requestNode node;
    node.source = source;
    node.destination = dest;
    node.fileDesc = fileDesc;
    node.next = NULL;
    return node;
}
#endif