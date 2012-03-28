#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"hashTable.h"

static void rehash();
static int getHashValue();

void hashAdd(hashTable *theTable, char *word){
	node *temp;
	int hashValue = getHashValue(word, theTable->size);
	
	//hash table is empty at this spot and new node can just be entered
	if(theTable->hTable[hashValue] == NULL){
		theTable->hTable[hashValue] = (node*)malloc(sizeof(node));
		if(theTable->hTable[hashValue] == NULL){
			fprintf(stderr, "Error allocating memory");
			exit(1);
		}
		theTable->hTable[hashValue]->word = word;
		theTable->hTable[hashValue]->count = 1;
		theTable->hTable[hashValue]->nextNode = NULL;
		theTable->count += 1;
	//if spot not empty than just loop to check doesnt match another node
	//if not just tacks new node to the end of the linked list
	}else{
		temp = theTable->hTable[hashValue];
		while(temp != NULL){
			if(!strcmp(word, temp->word)){
				temp->count += 1;
				break; //break here to skip uneeded checks
			}else if(temp->nextNode == NULL){
				temp->nextNode = (node*)malloc(sizeof(node));
				if(temp->nextNode == NULL){
					fprintf(stderr, "Error allocating memory");
					exit(1);
				}
				temp = temp->nextNode;
				temp->word = word;
				temp->count = 1;
				temp->nextNode = NULL;
				theTable->count += 1;
				temp = temp->nextNode;
			}else{
				temp = temp->nextNode;
			}
		}
	}
	//check to see if amount of nodes is greater than number of buckets
	//if so rehashes the table
	if(theTable->count > theTable->size){
		rehash(theTable);
	}
	return;
}


static void rehash(hashTable *theTable){
	int oldSize = theTable->size;
	int newSize = oldSize * 3;//increase the size of hash by 3 times
	node *temp1, *temp2;
	node **temp_hTable = theTable->hTable;//keep old table till everthing is rehashed
	theTable->hTable = (node**)malloc(sizeof(node*) * newSize);//new table created
	if(theTable->hTable == NULL){
		fprintf(stderr, "Error allocating memory");
		exit(1);
	}
	theTable->size = newSize;
	//loop through find every node and rehash it into new table
	for(int i = 0; i < oldSize; i++){
		if(temp_hTable[i] != NULL){
			temp1 = temp_hTable[i];
			temp2 = temp1;
			while(temp2 != NULL){
				temp2 = temp1->nextNode;
				int hashValue = getHashValue(temp1->word, theTable->size);
				//spot is empty just inserts it in that spot
				if(theTable->hTable[hashValue] == NULL){
					theTable->hTable[hashValue] = temp1;
					//nulling out nextNode so that it no longer links to old node
					temp1->nextNode = NULL;
				}else{//spot not empty just tacks on to end of link list
					node *temp3 = theTable->hTable[hashValue];
					while(temp3->nextNode != NULL){
						temp3 = temp3->nextNode;
					}
					temp3->nextNode = temp1;
					temp1->nextNode = NULL;
				}
				temp1 = temp2;
			}
			//setting all pointers in old table to null as those nodes
			//are switched to new table
			temp_hTable[i] = NULL;
		}
	}
	free(temp_hTable);
}

//modified version of hash function found @ www.cse.yorku.ca/~oz/hash.html basically hash = (hash - 1) * 33 + c
static int getHashValue(char *word, int size){
	unsigned long hash = 5381;
	for(int i = 0; word[i] != '\0'; i++){
		hash = ((hash << 5) + hash) + word[i];
	} 
	hash = hash % size;
	return hash;
}

//inits hash table setting begining size of 30
hashTable *hashInit(){
	hashTable *temp = (hashTable*) malloc(sizeof(hashTable));
	if(temp == NULL){
		fprintf(stderr, "Error allocating memory");
		exit(1);
	}
	temp->size = 30;
	temp->count = 0;
	temp->hTable = (node**)malloc(sizeof(node*) * 30);
	if(temp->hTable == NULL){
		fprintf(stderr, "Error allocating memory");
		exit(1);
	}
	for(int i = 0; i < 30; i++)
		temp->hTable[i] = NULL;
	return temp;	
}
