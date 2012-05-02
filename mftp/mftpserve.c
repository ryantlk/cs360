#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORT_NUMBER 49999

int makeconnection(int portnum);
int isregular(char *path);
void rcd(char *path, int controlfd);

int main(int argc, char *argv[]){
	int listenfd;
	int parentpid;
	int controlfd;
	int datalistenfd;
	int datafd;
	int length = sizeof(struct sockaddr_in);
	char *hostName;
	struct hostent *hostEntry;
	struct sockaddr_in clientAddr;
	
	listenfd = makeconnection(PORT_NUMBER);
	listen(listenfd, 4);

	while(1){
		waitpid(-1, NULL, WNOHANG);
		if((controlfd = accept(listenfd, (struct sockaddr*) &clientAddr, &length)) < 0){
			perror("Error with accept call");
			exit(EXIT_FAILURE);
		}
		if((parentpid = fork())){
			close(controlfd);
		}else{
			parentpid = getppid();
			close(listenfd);
			break;
		}
	}

	if((hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL){
		fprintf(stderr, "Error getting hostname");
	}else{
		hostName = hostEntry->h_name;
		printf("Child %d: Accepted connection from host %s\n", parentpid, hostName);
	}

	while(1){
		char command[1];
		char path[4096] = {0};
		read(controlfd, command, 1);
		if(command[0] == 'D'){
			read(controlfd, command, 1);
			char datacommand[10] = {0};
			struct sockaddr_in data;
			struct sockaddr_in client1Addr;
			int portnum;
			int childpid;
			int length1 = sizeof(struct sockaddr_in);
			
			memset(&data, 0, length);
			datalistenfd = makeconnection(0);
			listen(datalistenfd, 1);
			getsockname(datalistenfd,(struct sockaddr*) &data, &length1);
			portnum = ntohs(data.sin_port);
			sprintf(&datacommand[0], "A%d\n\0", portnum);
			write(controlfd, datacommand, strlen(&datacommand[0]));

			if((datafd = accept(datalistenfd, (struct sockaddr*) &client1Addr, &length)) < 0){
				perror("Error with accept call");
				exit(EXIT_FAILURE);
			}
			if((childpid = fork())){
				wait(NULL);
				close(datafd);
			}else{
				break;
			}
			
		}else if(command[0] == 'C'){
			int bytes;
			int i = 0;
			while((bytes = read(controlfd, command, 1)) > 0){
				if(command[0] == '\n'){
					path[i] = '\0';
					break;
				}else{
					path[i] = command[0];
					i++;
				}
			}
			rcd(path, controlfd);
			printf("Child %d: Changed current director to %s\n", parentpid, path);
		}else if(command[0] == 'L'){
			write(controlfd, "ENo data connection made\n", 25);
		}else if(command[0] == 'G'){
			write(controlfd, "ENo data connection made\n", 25);
		}else if(command[0] == 'P'){
			write(controlfd, "ENo data connection made\n", 25);
		}else if(command[0] == 'Q'){
			write(controlfd, "A\n", 2);
			close(controlfd);
			printf("Child %d: Quitting\n", parentpid);
			exit(EXIT_SUCCESS);
		}

	}

	while(1){
		char command[1];
		int pid;
		char path[4096] = {0};
		char response[256] = {0};
		read(controlfd, command, 1);
		if(command[0] == 'L'){
			write(controlfd, "A\n", 2);
			read(controlfd, command, 1);
			if(pid = fork()){
				wait(NULL);
				close(datafd);
				exit(EXIT_SUCCESS);
			}else{
				close(1);
				dup2(datafd, 1);
				close(datafd);
				close(controlfd);
				execlp("ls", "ls", "-l",  NULL);
			}
		}else if(command[0] == 'G'){
			int bytes;
			int filefd;
			int i = 0;
			char filebuf[4096] = {0};
			while((bytes = read(controlfd, command, 1)) > 0){
				if(command[0] == '\n'){
					path[i] = '\0';
					break;
				}else{
					path[i] = command[0];
					i++;
				}
			}
			if((filefd = open(path, O_RDONLY)) < 0){
				sprintf(&response[0], "E%s\n\0", strerror(errno));
				write(controlfd, response, strlen(response));
				exit(EXIT_SUCCESS);
			}
			if(isregular(path)){
				write(controlfd, "A\n", 2);
				printf("Child %d: Reading file %s\n", parentpid, path);
				printf("Child %d: Transmitting file %s to client\n", parentpid, path);
				while((bytes = read(filefd, filebuf, 4096)) > 0){
					write(datafd, filebuf, bytes);
				}
				close(datafd);
				close(filefd);
				exit(EXIT_SUCCESS);
			}else{
				write(controlfd, "EFile not regular\n", 18);
				close(filefd);
				close(datafd);
				exit(EXIT_SUCCESS);
			}	
		}else if(command[0] == 'P'){
			int bytes;
			int filefd;
			int i = 0;
			char filebuf[4096] = {0};
			while((bytes = read(controlfd, command, 1)) > 0){
				if(command[0] == '\n'){
					path[i] = '\0';
					break;
				}else{
					path[i] = command[0];
					i++;
				}
			}
			printf("Child %d: Writing file %s\n", parentpid, path);
			if((filefd = open(path, O_CREAT | O_APPEND | O_EXCL | O_WRONLY,
					 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
				sprintf(&response[0], "E%s\n\0", strerror(errno));
				write(controlfd, response, strlen(response));
				close(datafd);
				exit(EXIT_SUCCESS);
			}
			write(controlfd, "A\n", 2);
			printf("Child %d: Receiving file %s from client\n", parentpid, path);
			while((bytes = read(datafd, filebuf, 4096)) > 0){
				write(filefd, filebuf, bytes);
			}
			close(filefd);
			close(controlfd);
			exit(EXIT_SUCCESS);
		}else{
			puts("unkown command ignored");
			close(datafd);
			exit(EXIT_SUCCESS);
		}
	}
	
}

int makeconnection(int portnum){
	int listenfd;
	int length = sizeof(struct sockaddr_in);
	struct sockaddr_in servAddr;
	
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portnum);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error with bind call");
		exit(EXIT_FAILURE);
	}
	if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
		perror("Error with bind call");
		exit(EXIT_FAILURE);
	}
	return listenfd;
}

void rcd(char *path, int controlfd){
	char *errormessage;
	char clientmessage[256] = {0};
	if(chdir(path) < 0){
		errormessage = strerror(errno);
		clientmessage[0] = 'E';
		clientmessage[1] = '\0';
		strcat(&clientmessage[0], errormessage);
		strcat(&clientmessage[0], "\n");
		write(controlfd, clientmessage, strlen(&clientmessage[0]));
	}else{
		write(controlfd, "A\n", 2);
	}
}

int isregular(char *path){
	struct stat area, *s = &area;
	return (lstat(path, s) == 0) && S_ISREG(s->st_mode);
}
