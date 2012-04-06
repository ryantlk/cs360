#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORT_NUMBER 49999

int main(int argc, char *argv[]){
	int socketfd;
	char date[100];
	struct sockaddr_in servAddr;
	struct hostent *hostEntry;
	struct in_addr **pptr;

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
	read(socketfd, date, 100);
	for(int i = 0; i < 20; i++){
		printf("%c", date[i]);
	}
	puts("");
	close(socketfd);

	return 0;
}
