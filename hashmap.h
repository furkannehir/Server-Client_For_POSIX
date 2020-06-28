#ifndef HASHMAP
#define HASHMAP


#include<stdlib.h>
#include<string.h>



typedef struct hashNode{
    char key[100];
    char value[5000];
}hashNode;

typedef struct hashmap{
    hashNode * array;
    int size;
    int capacity;
    int * keyList;
}hashmap;

hashmap CreateHashmap();
void DestroyHashmap(hashmap * map);
int hashCode(hashmap * map, char * key);
hashNode GetItemHashmap(hashmap * map, char * key);
void InsertHashmap(hashmap * map, char * key, char * value);
hashmap MakeItLarger(hashmap * map);



hashmap CreateHashmap()
{
    hashmap map;
    map.capacity = 100;
    map.size = 0;
    map.array = (hashNode*)calloc(map.capacity, sizeof(hashNode));
    map.keyList = (int*)calloc(map.capacity, sizeof(int));
    return map;
}

void DestroyHashmap(hashmap * map)
{
    free(map->array);
    free(map->keyList);
}

int hashCode(hashmap * map, char * key)
{
    int len = strlen(key);
    int i, hashcode = 0;
    for(i = 0; i < len; ++i)
        hashcode += key[i];
    return hashcode/map->capacity;
}

hashNode GetItemHashmap(hashmap * map, char * key)
{
    hashNode node;
    strcpy(node.key, "");
    strcpy(node.value, "");
    int hash = hashCode(map, key);
    while(strcmp(map->array[hash].key,""))
    {
        if(!strcmp(map->array[hash].key, key))
            return map->array[hash];
        ++hash;
    }
    return node;
}

void InsertHashmap(hashmap * map, char * key, char * value)
{
    if(map->size >= map->capacity/2)
        *map = MakeItLarger(map); 
    int hash = hashCode(map, key);
    while(strcmp(map->array[hash].key,""))
    {
        ++hash;
        if(hash == map->capacity)
        {
            hash = 0;
        }
    }
    strcpy(map->array[hash].key, key);
    strcpy(map->array[hash].value, value);
    map->keyList[map->size] = hash;
    ++map->size;
}

hashmap MakeItLarger(hashmap * map)
{
    hashmap newMap;
    newMap.capacity = map->capacity * 2;
    newMap.size = 0;
    newMap.array = (hashNode*)calloc(newMap.capacity*2, sizeof(hashNode));
    newMap.keyList = (int*)calloc(newMap.capacity*2, sizeof(int));
    int i, hash;
    for(i = 0; i < map->size; ++i)
    {
        hash = map->keyList[i];
        InsertHashmap(&newMap, map->array[hash].key, map->array[hash].value);
    }
    return newMap;
}

#endif