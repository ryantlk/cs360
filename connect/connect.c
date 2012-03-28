#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>

//stores command and args list for commands that need to be execd
typedef struct{
	char *command;
	char **args;
}prog;

//counts :'s to get number of commands
int getNumProgs(char *argv[]){
	int i = 0;
	int progCount = 1;
	while(argv[i] != NULL){
		if(!strcmp(argv[i], ":")){
			progCount++;
		}
		i++;
	}
	return progCount;
}

void getArgCounts(int argsPerProg[], int count, char *argv[]){
	int j = 1;
	for(int i = 0; i < count; i++){
		//starting at 1 to allow space for null
		argsPerProg[i] = 1;
		while(argv[j] != NULL && strcmp(argv[j], ":")){
			argsPerProg[i] += 1;
			j++;
		}
		//move past colon for next loop
		j++;
	}
}

void connect(int n, prog **theProgs){
	int fd[2];
	int rdr, wtr;
	pipe(fd);
	rdr = fd[0];
	wtr = fd[1];
	if(fork()){
		close(wtr);
		dup2(rdr, 0);
		execvp(theProgs[n]->command, theProgs[n]->args);
		fprintf(stderr,"Error with exec: %s\n", strerror(errno));
	}else{
		close(rdr);
		dup2(wtr, 1);
		if(n > 0){
			//more processes left to fork
			connect(n-1, theProgs);
		}else{
			//if we get here this process has no purpose
			//could fix this worthless process later
			exit(0);
		}
	}
}

//mallocs prog struct array and fills with the needed data
prog **progsInit(int count, int argsPerProg[], char *argv[]){
	int j = 1;
	prog **tmp = (prog**)malloc(sizeof(prog*) * (count + 1));
	for(int i = 0; i < count; i++){
		tmp[i] = (prog*)malloc(sizeof(prog));
		tmp[i]->args = (char**)malloc(sizeof(char*) * argsPerProg[i]);
		tmp[i]->command = strdup(argv[j]);
		for(int k = 0; k < argsPerProg[i] - 1; k++){
			tmp[i]->args[k] = strdup(argv[j]);
			j++;
		}
		tmp[i]->args[argsPerProg[i]] = NULL;
		j++;
	}
	tmp[count] = NULL;
	return tmp;
}

int main(int argc, char *argv[]){
	int count = getNumProgs(argv);
	int argsPerProg[count];
	getArgCounts(argsPerProg, count, argv);
	prog **theProgs = progsInit(count, argsPerProg, argv);
	connect(count - 1, theProgs);
	return 0;
}
