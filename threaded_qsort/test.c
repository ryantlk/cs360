#include<stdio.h>
#include<stdlib.h>
#include"getWord.h"
#include"sort.h"
#include<time.h>

int main(){
	char *words[1000];
	char *words2[1000];
	FILE *fp = fopen("kjvbible.txt", "r");
	for(int i = 0; i < 1000; i++){
		words[i] = words2[i] =  getNextWord(fp);
	}
	clock_t start = clock();
	setSortThreads(2);
	sortThreaded(&words[0], 1000);
	clock_t end = clock();
	unsigned long time = (end - start) * 1000 / CLOCKS_PER_SEC;
	printf("%d\n", (int) start);
	return 0;
}
