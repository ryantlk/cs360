#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/stat.h>
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
int isregular(char *path);
void printerror(char *errormessage);
void cd(char *path);
void rcd(char *path, int controlfd);
void ls();
void rls(int rlssocket, int controlfd);
void show(char *path, int showsocket, int controlfd);
void get(char *path, int filefd, int getsocket, int controlfd);
void put(char *path, int filefd, int putsocket, int controlfd);

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
			if((portnum = getportnum(controlfd)) < 0){	
				printerror("rls not executed due to error");
				continue;
			}
			if((rlssocket = makeconnection(argv[1], portnum)) < 0){
				printerror("rls not executed due to error");
				continue;
			}
			puts("here");
			rls(rlssocket, controlfd);
			close(rlssocket);
		}else if(!strcmp(token, "cd")){
			token = strtok(NULL, delims);
			cd(token);
		}else if(!strcmp(token, "rcd")){
			token = strtok(NULL, delims);
			rcd(token, controlfd);
		}else if(!strcmp(token, "get")){
			int portnum;
			int getsocket;
			int filefd;
			token = strtok(NULL, delims);
			char *filenamedelims = "/";
			char *filename = strtok(token, filenamedelims);
			char *lastname = filename;
			while((filename = strtok(NULL, filenamedelims)) != NULL){
				lastname = filename;
			}
			if((filefd = open(lastname, O_CREAT | O_APPEND | O_EXCL | O_WRONLY, S_IRUSR 
										| S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
				perror("Error opening file");
				continue;
			}
			if((portnum = getportnum(controlfd)) < 0){
				printerror("get not executed due to error");
				continue;
			}
			if((getsocket = makeconnection(argv[1], portnum)) < 0){
				printerror("get not executed due to error");
				continue;
			}
			get(token, filefd, getsocket, controlfd);
			close(filefd);
			close(getsocket);
		}else if(!strcmp(token, "show")){
			int portnum;
			int showsocket;
			token = strtok(NULL, delims);
			if((portnum = getportnum(controlfd)) < 0){
				printerror("show not executed due to error");
				continue;
			}
			if((showsocket = makeconnection(argv[1], portnum)) < 0){
				printerror("show not executed due to error");
				continue;
			}
			show(token, showsocket, controlfd);
			close(showsocket);
		}else if(!strcmp(token, "put")){
			int portnum;
			int putsocket;
			int filefd;
			token = strtok(NULL, delims);
			char *filenamedelims = "/";
			char *filename = strtok(token, filenamedelims);
			char *lastname = filename;
			while((filename = strtok(NULL, filenamedelims)) != NULL){
				lastname = filename;
			}	
			if((filefd = open(token, O_RDONLY)) < 0){
				perror("Error opening file");
				continue;
			}
			if(isregular(token)){
				if((portnum = getportnum(controlfd)) < 0){
					printerror("put not executed due to error");
					continue;
				}
				if((putsocket = makeconnection(argv[1], portnum)) < 0){	
					printerror("put not executed due to error");
					continue;
				}
				put(lastname, filefd, putsocket, controlfd);
				close(filefd);
				close(putsocket);
			}else{
				close(filefd);
				printerror("File not regular put not executed");
			}
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
	char *command = "D\n";
	char response[MAX_ACKNOWLEDGE_SIZE] = {0};
	write(controlfd, command, 2);
	read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		printf("%s\n", &response[1]);
		return -1;
	}else{
		sscanf(response, "A%d\n", &portnum);
	}
	return portnum;
}

void rls(int rlssocket, int controlfd){
	char response[MAX_ACKNOWLEDGE_SIZE] = {0};
	write(controlfd, "L\n", 2);
	read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		printf("%s", &response[1]);
	}else{
		int pid;
		if((pid = fork())){
			wait(NULL);
		}else{
			close(0);
			dup2(rlssocket, 0);
			close(rlssocket);
			execlp("more", "more", "-20", NULL);
		}
	}
	return;
}

void show(char *path, int showsocket, int controlfd){
	char response[MAX_ACKNOWLEDGE_SIZE] = {0};
	char *command = (char*)calloc(256, sizeof(char));
	command[0] = 'G';
	strcat(command, path);
	strcat(command, "\n");
	write(controlfd, command, strlen(command));
	read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		printf("%s", &response[1]);
	}else{
		int pid;
		if((pid = fork())){
			wait(NULL);
		}else{
			close(0);
			dup2(showsocket, 0);
			close(showsocket);
			execlp("more", "more", "-20", NULL);
		}
	}
	free(command);
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
	char response[MAX_ACKNOWLEDGE_SIZE] = {0};
	command[0] = 'C';
	strcat(command, path);
	strcat(command, "\n");
	write(controlfd, command, strlen(command));
	read(controlfd, response, MAX_ACKNOWLEDGE_SIZE);
	if(response[0] == 'E'){
		printf("%s", &response[1]);
	}
	free(command);
}

void get(char *path, int filefd, int getsocket, int controlfd){
	int bytes;
	char *filebuf = (char*)calloc(4096, sizeof(char));
	char response[256] = {0};
	write(controlfd, "G", 1);
	write(controlfd, path, strlen(path));
	write(controlfd, "\n", 1);
	read(controlfd, response, 256);
	if(response[0] == 'E'){
		printf("%s", &response[1]);
	}else{
		while((bytes = read(getsocket, filebuf, 4096)) > 0){
			write(filefd, filebuf, bytes);
		}
	}
	free(filebuf);
	return;
}

int isregular(char *path){
	struct stat area, *s = &area;
	return (lstat(path, s) == 0) && S_ISREG(s->st_mode);
}

void put(char *path, int filefd, int putsocket, int controlfd){
	int bytes;
	char *filebuf = (char*)calloc(4096, sizeof(char));
	char response[256] = {0};
	write(controlfd, "P", 1);
	write(controlfd, path, strlen(path));
	write(controlfd, "\n", 1);
	read(controlfd, response, 256);
	if(response[0] == 'E'){
		printf("%s", &response[1]);
	}else{
		while((bytes = read(filefd, filebuf, 4096)) > 0){
			write(putsocket, filebuf, bytes);
		}
	}
	free(filebuf);
	return;
}
