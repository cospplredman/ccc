#ifndef CCC_HASHTABLE_H
#define CCC_HASHTABLE_H
#include<stddef.h>

typedef struct HashTable{
	size_t size;
	char data[];
} HashTable;

HashTable *allocHashTable(size_t size, size_t itemsz);
void freeHashTable(HashTable *table);
void *getTablePtr_(HashTable *table, void *val, size_t itemsz, size_t hash, int (*cmpfn)(void*,void*));
#define getTablePtr(t,v,h,f) (typeof(v))getTablePtr_((t),(v),sizeof(*(v)),(h),(f))
void mapHashTable(HashTable *table, size_t itemsz, void (*fn)(void*));
#endif
