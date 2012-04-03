#include<stdio.h>
#include<stdlib.h>
#include"getWord.h"
#include"sort.h"
#include<time.h>
#include<sys/time.h>
#include<string.h>

#define SIZE 10000

static int p_sortstring(const void *str1, const void *str2){
	const char **rec1 = (const char **)str1;
	const char **rec2 = (const char **)str2;
	return strcmp(*rec1, *rec2);
}

int main(){
	struct timeval start, end;
	char *words[SIZE];
	char *words2[SIZE];
	FILE *fp = fopen("kjvbible.txt", "r");
	for(int i = 0; i < SIZE; i++){
		words[i] = words2[i] =  getNextWord(fp);
	}

	setSortThreads(7);
	gettimeofday(&start, NULL);
	sortThreaded(&words[0], SIZE);
	gettimeofday(&end, NULL);
	double ts = start.tv_usec;
	double te = end.tv_usec;
	printf("%lf\n", (te - ts) / 1000000 );

	gettimeofday(&start, NULL);
	qsort(words2, SIZE, sizeof(char*), p_sortstring);
	gettimeofday(&end, NULL);
	ts = start.tv_usec;
	te = end.tv_usec;
	printf("%lf\n", (te - ts) / 1000000 );
	return 0;
}
