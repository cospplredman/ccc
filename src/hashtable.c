#include<stdlib.h>
#include"hashtable.h"

HashTable*
allocHashTable(size_t size, size_t itemsz)
{
	HashTable *table = malloc(sizeof(HashTable) + itemsz*size);
	table->size = size;
	return table;
}

void
freeHashTable(HashTable *table)
{
	free(table);
}

void*
getTablePtr_(HashTable *table, void *val, size_t itemsz, size_t hash, int (*cmpfn)(void*, void*))
{
	void* item;
	do item = table->data + (hash++ % table->size) * itemsz;
	while(!cmpfn(val, item));
	return item;
}

void
mapHashTable(HashTable *table, size_t itemsz, void (*fn)(void*))
{
	void *end = table->data + table->size * itemsz;
	for(void *item = table->data; item < end; item += itemsz)
		fn(item);
}

