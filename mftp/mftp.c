#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORT_NUMBER			49999
#define MAX_COMMAND_SIZE	4096

int hasargs(char *string);

int main(int argc, char *argv[]){
	int socketfd;
	struct sockaddr_in servAddr;
	struct hostent *hostEntry;
	struct in_addr **pptr;
	char *command = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *tokenize = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *token = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *delims = " \n";
/*
	if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error with socket call");
		exit(EXIT_FAILURE);
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT_NUMBER);

	if((hostEntry = gethostbyname(argv[1])) == NULL){
		fprintf(stderr, "Error with gethostbyname call");
		exit(EXIT_FAILURE);
	}
	pptr = (struct in_addr**) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

	if((connect(socketfd, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0){
		perror("Error with connect call");
		exit(EXIT_FAILURE);
	}
*/
	while(1){
		printf("mftp>");
		fgets(command, sizeof(char) * MAX_COMMAND_SIZE, stdin);
		if(!hasargs(command)){
			continue;	
		}
		memcpy(tokenize, command, sizeof(char) * MAX_COMMAND_SIZE);
		token = strtok(tokenize, delims);

		if(!strcmp(token, "ls")){
			system("ls -l | more -20");
		}else if(!strcmp(token, "rls")){

		}else if(!strcmp(token, "cd")){
			token = strtok(NULL, delims);
			if(chdir(token) < 0){
				perror("Change directory");
			}
		}else if(!strcmp(token, "rcd")){

		}else if(!strcmp(token, "get")){

		}else if(!strcmp(token, "show")){

		}else if(!strcmp(token, "put")){

		}else if(!strcmp(token, "exit")){
			close(socketfd);
			exit(EXIT_SUCCESS);
		}
	}
}

int hasargs(char *string){
	char *tmp = string;
	while(*tmp != '\n' && isspace(*tmp)){
		++tmp;
	}
	if(*tmp == '\n'){
		return 0;
	}else{
		return 1;
	}
}
