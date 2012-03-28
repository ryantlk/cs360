#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include"random.h"

#define THINK_MEAN 		11
#define THINK_STDEV 	7
#define EAT_MEAN 		9
#define EAT_STDEV 		3
#define NUM_CHOPS		5

int philosiphise(int id, int philnum){
	int think, eat;
	int eattot = 0;
	int thinktot = 0;
	srand(getpid());
	struct sembuf lock[2] = {{philnum, -1, 0}, {(philnum + 1) % NUM_CHOPS, -1, 0}};
	struct sembuf unlock[2] = {{philnum, 1, 0}, {(philnum + 1) % NUM_CHOPS, 1, 0}};
	while(eattot < 100){
		think = randomGaussian(THINK_MEAN, THINK_STDEV);
		if(think < 0){
			think = 0;
		}
		thinktot += think;
		printf("philosopher %d thinking for %d seconds. (total = %d)\n", philnum, think, thinktot);
		sleep(think);
		eat = randomGaussian(EAT_MEAN, EAT_STDEV);
		if(eat < 0){
			eat = 0;
		}
		if(semop(id, &lock[0], 2) == -1){
			fprintf(stderr, "Error locking: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		eattot += eat;
		printf("philosopher %d eating for %d seconds. (total = %d)\n", philnum, eat, eattot);
		sleep(eat);
		if(semop(id, &unlock[0], 2) == -1){
			fprintf(stderr, "Error unlocking: %s\n", strerror(errno));
		}
	}
	exit(EXIT_SUCCESS);	
}

int main(int argc, char *argv[]){
	int id;
	int philosopher = 0;
	id = semget(IPC_PRIVATE, NUM_CHOPS, S_IRUSR | S_IWUSR);
	if(id < 0){
		fprintf(stderr, "Error with semget");
		exit(EXIT_FAILURE);
	}
	for(int i = 0; i < 5; i++){
		struct sembuf set = {i, 1, 0};
		semop(id, &set, 1);
	}
	for(int i = 0; i < 5; i++){
		if(!fork()){
			philosiphise(id, philosopher);
		}
		philosopher++;
	}
	for(int i = 0; i < 5; i++){
		wait(NULL);
	}
	semctl(id, 0, IPC_RMID, 0);
	return 0;
}
