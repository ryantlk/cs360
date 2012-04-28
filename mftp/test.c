#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(){
	char *command = (char*)calloc(4096, sizeof(char));
	char *tokenize = (char*)calloc(4096, sizeof(char));
	char *token = (char*)calloc(4096, sizeof(char));
	char delims[] = " ";
	while(1){
		fgets(command, sizeof(char) *  4096, stdin);
		memcpy(tokenize, command, sizeof(char) * 4096);
		token = strtok(tokenize, delims);
		printf("token is: %s\n", token);
		while((token = strtok(NULL, delims)) != NULL){
			printf("token is: %s\n", token);
		}
		if(!strcmp(command, "exit\n")){
			return 1;
		}
	}

}
