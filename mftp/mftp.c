#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORT_NUMBER				49999
#define MAX_COMMAND_SIZE		4096
#define MAX_ACKNOWLEDGE_SIZE	256

int hasargs(char *string);
int makeconnection(char *hostname, int portnum);
int getportnum(int controlfd);
void printerror(char *errormessage);
void cd(char *path);
void rcd(char *path, int controlfd);
void ls();
void rls(int rlssocket, int controlfd);

int main(int argc, char *argv[]){
	//check for correct args
	if(argc < 2){
		printerror("Error to few arguments: ");
		exit(EXIT_FAILURE);
	}

	int controlfd = makeconnection(argv[1], PORT_NUMBER);
	char *command = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *tokenize = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *token = (char*)calloc(MAX_COMMAND_SIZE, sizeof(char));
	char *delims = " \n\t";

	while(1){
		printf("mftp>");
		fgets(command, sizeof(char) * MAX_COMMAND_SIZE, stdin);
		if(!hasargs(command)){
			continue;	
		}
		memcpy(tokenize, command, sizeof(char) * MAX_COMMAND_SIZE);
		token = strtok(tokenize, delims);

		if(!strcmp(token, "ls")){
			ls();
		}else if(!strcmp(token, "rls")){
			int portnum;
			int rlssocket;
			char *buf = (char*)calloc(MAX_ACKNOWLEDGE_SIZE, sizeof(char));
			if((portnum = getportnum(controlfd)) < 0){	
				printerror("rls not executed due to error");
				continue;
			}
			if((rlssocket = makeconnection(argv[1], portnum)) < 0){
				printerror("rls not executed due to error");
				continue;
			}
			rls(rlssocket, controlfd);
			close(rlssocket);
			free(buf);
		}else if(!strcmp(token, "cd")){
			token = strtok(NULL, delims);
			cd(token);
		}else if(!strcmp(token, "rcd")){
			token = strtok(NULL, delims);
			rcd(token, controlfd);
		}else if(!strcmp(token, "get")){

		}else if(!strcmp(token, "show")){

		}else if(!strcmp(token, "put")){

		}else if(!strcmp(token, "exit")){
			char *quit = "Q\n";
			char buf[100];
			write(controlfd, quit, sizeof(char) * 2);
			read(controlfd, buf, 100); 
			close(controlfd);
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

void ls(){
	system("ls -l | more -20");
	return;
}

void cd(char *path){
	if(chdir(path) < 0){
		perror("Change directory");
	}
	return;
}

int getportnum(int controlfd){
	int portnum;
	int bytes;
	char *command = "D\n";
	char *response = (char*)calloc(MAX_ACKNOWLEDGE_SIZE, sizeof(char));
	write(controlfd, command, 2);
	bytes = read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		++response;
		printf("%s", response);
		free(response);
		return -1;
	}else{
		sscanf(response, "A%d\n", &portnum);
	}
	free(response);
	return portnum;
}

void rls(int rlssocket, int controlfd){
	char *response = (char*)calloc(MAX_ACKNOWLEDGE_SIZE, sizeof(char));
	write(controlfd, "L\n", 2);
	read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		++response;
		printf("%s", response);
	}else{
		int pid;
		if(pid = fork()){
			wait(NULL);
		}else{
			close(0);
			dup2(rlssocket, 0);
			close(rlssocket);
			free(response);
			execlp("more", "more", "-20", NULL);
		}
	}
	free(response);
	return;
}

int makeconnection(char *hostname, int portnum){
	struct sockaddr_in servAddr;
	struct hostent *hostEntry;
	struct in_addr **pptr;
	int controlfd;

	if((controlfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error with socket call");
		return -1;
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portnum);

	if((hostEntry = gethostbyname(hostname)) == NULL){
		fprintf(stderr, "Error with gethostbyname call");
		return -1;
	}
	pptr = (struct in_addr**) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

	if((connect(controlfd, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0){
		perror("Error with connect call");
		return -1;
	}
	return controlfd;
}

void printerror(char *errormessage){
	strcat(errormessage, "\0");
	fprintf(stderr, "%s\n", errormessage);
	return;
}

void rcd(char *path, int controlfd){
	char *command = (char*)calloc(4096, sizeof(char));
	command[0] = 'C';
	strcat(command, 
}
