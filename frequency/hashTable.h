#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct node{
	char *word;
	int count;
	struct node *nextNode;
}node;

typedef struct{
	int size;
	int count;
	node **hTable;
}hashTable;

//hashAdd takes a hashTable struct and a char* and adds that char* to the
//hashtable
void hashAdd(hashTable *theTable, char *word);
//inits hash table setting beginning values. hash table starts with initial
//size of 30 buckets
hashTable *hashInit();

#endif
