#include<stdio.h>
#include<stdlib.h>
#include<getWord.h>
#include"hashTable.h"

void printwords(hashTable *theTable, int wordsToPrint);
int struc_cmp(const void *a, const void *b);

int main(int argc, char *argv[]){
	int wordsToPrint; //#of words to print
	int argP;

	//check # of words arg was givin if not sets value to -1 meaning all words
	if(argv[1][0] == '-'){
		wordsToPrint = (int) strtol(argv[1]+1, NULL, 10);
		argP = 2;
	}
	else{
		wordsToPrint = -1;
		argP = 1;
	}

	hashTable *theTable = hashInit();

	//loop opens files in order and hash's words
	for(; argP < argc; argP++){
		FILE *fp = fopen(argv[argP], "r");

		if(fp == NULL){
			fprintf(stderr, "Error opening file \"%s\" no words counted from file.\n", argv[argP]);
			continue;
		}

		char *wordToHash = getNextWord(fp);

		while(wordToHash != NULL){
			hashAdd(theTable, wordToHash);
			wordToHash = getNextWord(fp);	
		}

		fclose(fp);

	}

	printwords(theTable, wordsToPrint);
	free(theTable);
	return 0;
}

void printwords(hashTable *theTable, int wordsToPrint){
	unsigned int position = 0;
	node *temp1, *temp2;
	node **printTable = (node**)malloc(sizeof(node*) * theTable->count);
	if(printTable == NULL){
		fprintf(stderr, "Error allocating memory");
		exit(1);
	}
	//loop finds all nodes in given hashtable and moves them to array
	//printTable to be sorted and printed
	for(int i = 0; i < theTable->size; i++){
		if(theTable->hTable[i] != NULL){
			temp1 = theTable->hTable[i];
			temp2 = temp1;
			while(temp2 != NULL){
				temp2 = temp1->nextNode;
				printTable[position] = temp1;
				temp1->nextNode = NULL;
				position++;
				temp1 = temp2;
			}
		}
	}
	//sorts array of nodes
	qsort(printTable, theTable->count, sizeof(*printTable), struc_cmp);
	//check if no arg was given for amount of words or if arg was larger than
	//number of words if either is true just prints all words also frees nodes
	//as they are printed
	if(wordsToPrint == -1 || wordsToPrint > theTable->count)
		wordsToPrint = theTable->count;
	for(int i = 0; i < wordsToPrint; i++){
		printf("%10d %s\n", printTable[i]->count, printTable[i]->word);
		free(printTable[i]);
	}
	free(printTable);	
}

int struc_cmp(const void *a, const void *b){
	node **ia = (node**)a;
	node **ib = (node**)b;
	//for typical qsort implementation would be ia - ib but i want descending
	//so i switch the order
	return (*ib)->count - (*ia)->count;
}
