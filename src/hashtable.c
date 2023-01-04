#include<stddef.h>
#include<stdlib.h>
#include"hashtable.h"

HashTable*
allocHashTable(size_t size, size_t itemsz)
{
	HashTable *table = malloc(sizeof(HashTable));
	table->val = malloc(size*itemsz);
	table->size = size;
	table->itemsz = itemsz;
	return table;
}

void
freeHashTable(HashTable *table)
{
	free(table->val);
	free(table);
}

void**
getTablePtr(HashTable *table, void *val, size_t hash, int (*cmpfn)(void*, void*))
{
	size_t retry = 0;
	do{
		size_t index = ((hash + retry) % table->size) * table->itemsz;
		if(cmpfn(val, (char*)table->val + index))
			return (void**)((char*)table->val + index);
		retry++;
	}while(retry < table->size);
	return NULL;
}
