#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>

#define WORD_SIZE 15
#define DICT_LINE_SIZE 16

int ok(int fd, char *word);

int main(int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "Incorrect number of arguments correct usage:\n"
						"%s (Desired Word) (Path to dictionary)\n", argv[0]);
		exit(1);
	}
	int found;
	char *dictPath = argv[2];
	char *word = (char*)malloc(WORD_SIZE * sizeof(char));
	for(int i = 0; i < WORD_SIZE; i++){
		word[i] = ' ';
	}
	for(int i = 0; i < strlen(argv[1]); i++){
		word[i] = argv[1][i];
	}
	int fd = open(dictPath, O_RDONLY);
	if(fd < 0){
		fprintf(stderr, "Error opening file \"%s\": %s.\n", dictPath, strerror(errno));
		exit(1);
	}
	found = ok(fd, word);
	printf("%s\n", (found == 1)?"yes":"no");
	free(word);
	return 0;
}

int ok(int fd, char *word){
	int top, mid, matches;
	int errCheck = 0;
	int bot = 0;
	char *have = (char*)malloc(sizeof(char) * WORD_SIZE);
	top = lseek(fd, 0, SEEK_END)/DICT_LINE_SIZE;
	while(!(bot >= top)){
		mid = (top + bot) >> 1;
		errCheck = lseek(fd, DICT_LINE_SIZE * mid, SEEK_SET);
		if(errCheck == -1){
			fprintf(stderr, "Error with lseek: %s.", strerror(errno));
			exit(1);
		}
		errCheck = read(fd, have, WORD_SIZE);
		if(errCheck == -1){
			fprintf(stderr, "Error reading from file: %s.", strerror(errno));
			exit(1);
		}
		matches = strncmp(word, have, WORD_SIZE);
		if(matches == 0){
			free(have);
			return 1;
		}else if(matches < 0){
			top = mid;
		}else if(matches > 0){
			bot = mid + 1;
		}
	}
	free(have);
	return 0;
}
