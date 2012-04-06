#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORT_NUMBER 49999

int main(int argc, char *argv[]){
	int listenfd;
	int connectfd;
	int length = sizeof(struct sockaddr_in);
	char *hostName;
	struct hostent *hostEntry;
	struct sockaddr_in clientAddr;
	struct sockaddr_in servAddr;

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT_NUMBER);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error with socket call");
		exit(EXIT_FAILURE);
	}
	if(bind(listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
		perror("Error with bind call");
		exit(EXIT_FAILURE);
	}
	listen(listenfd, 1);
	while(1){	
		waitpid(-1, NULL, WNOHANG);
		if((connectfd = accept(listenfd, (struct sockaddr*) &clientAddr, &length)) < 0){
			perror("Error with accept call");
			exit(EXIT_FAILURE);
		}
		if(fork()){
			close(connectfd);
		}else{
			close(listenfd);
			break;
		}
	}
	
	if((hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET)) == NULL){
		//herror(NULL);
		fprintf(stderr, "Error with gethostbyaddr");
		exit(EXIT_FAILURE);
	}

	hostName = hostEntry->h_name;
	printf("%s\n", hostName);
	
	time_t seconds;
	char *date;
	seconds = time(NULL);
	date = ctime(&seconds);
	write(connectfd, date, strlen(date));
	close(connectfd);
	
	return 0;
}
