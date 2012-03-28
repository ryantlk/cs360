#include<signal.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<ctype.h>

#define WTR 3
#define RDR 4
#define DFLT_DLY 5
#define BUF_SIZE 512

int chldpid;

void parentHandler(int code){
	signal(SIGINT, SIG_IGN);
	if(code == SIGINT){
	char *buf = (char*)calloc(sizeof(char), BUF_SIZE);
		printf("Enter single line message: ");
		fgets(buf, BUF_SIZE, stdin);
		//removing newline that fgets captures
		if(buf[(strlen(buf) - 1)] == '\n'){
			buf[(strlen(buf) - 1)] = '\0';
		}
		write(WTR, buf, strlen(buf));
		if(!strcmp(buf, "exit")){
			int status;
			kill(chldpid, SIGFPE);
			wait(&status);
			printf("Parent process exiting\n");
			exit(status);
		}
		kill(chldpid, SIGFPE);
		free(buf);
	}	
}

void childHandler(int code){
	signal(SIGINT, SIG_IGN);
	static int alrmTime;
	static char *prntMsg;
	if(code == SIGINT){
		signal(SIGALRM, SIG_IGN);
	}else if(code == SIGFPE){
		char *buf = (char*)calloc(sizeof(char), BUF_SIZE);
		char *temp;
		read(RDR, buf, BUF_SIZE);
		if(!strcmp(buf, "exit")){
			printf("Child process exiting\n");
			exit(0);
		}
		alrmTime = (int)strtol(buf, &temp, 10);
		//remove any whitespace left in temp from strtol
		while(isspace(*temp)){
			++temp;
		}
		//have to special case the first calloc to avoid free attempt on non
		//calloc'd pointer
		if(prntMsg == NULL){
			prntMsg = (char*)calloc(sizeof(char), strlen(temp));			
			strcpy(prntMsg, temp);
		}else if(prntMsg != NULL){
			free(prntMsg);
			prntMsg = (char*)calloc(sizeof(char), strlen(temp));			
			strcpy(prntMsg, temp);	
		}
		if(!strcmp(temp, buf) || alrmTime < 1){
			alrmTime = DFLT_DLY;
		}
		free(buf);
		printf("%s\n", prntMsg);
		alarm(alrmTime);
		signal(SIGALRM, childHandler);
	}else if(code == SIGALRM){
		printf("%s\n", prntMsg);
		alarm(alrmTime);
	}
}

int main(int argc, char *argv[]){
	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];
	if((chldpid = fork()) != 0){
		close(rdr);
		dup2(wtr, WTR);
		while(1){
			signal(SIGINT, parentHandler);
			pause();
		}
	}else{
		close(wtr);
		dup2(rdr, RDR);
		while(1){
			signal(SIGFPE, childHandler);
			signal(SIGINT, childHandler);
			pause();
		}
	}
}
