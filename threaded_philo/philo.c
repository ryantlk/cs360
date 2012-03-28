#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include"random_r.h"

#define THINK_MEAN 	11
#define THINK_STDEV 7
#define EAT_MEAN 	9
#define EAT_STDEV 	3
#define NUM_CHOPS 	5
#define NUM_PHILOS 	5
#define RETRY_WAIT 	5

int chopsticks[NUM_CHOPS];
pthread_mutex_t chopstick_mutex;

void eat(int *eattot, int philnum){
	unsigned int state = (unsigned int)pthread_self();
	int eat = randomGaussian_r(EAT_MEAN, EAT_STDEV, &state);
	if(eat < 0){
		eat = 0;
	}
	*eattot += eat;
	printf("philosopher %d eating for %d seconds. (total = %d)\n", philnum, eat, *eattot);
	sleep(eat);
}

void think(int *thinktot, int philonum){
	unsigned int state = (unsigned int)pthread_self();
	int think = randomGaussian_r(THINK_MEAN, THINK_STDEV, &state);
	if(think < 0){
		think = 0;
	}
	*thinktot += think;
	printf("philosopher %d thinking for %d seconds. (total = %d)\n", philonum, think, *thinktot);
	sleep(think);
}

void philosophise(int *philnum){
	int eattot = 0;
	int thinktot = 0;
	think(&thinktot, *philnum);
	while(eattot < 100){
		pthread_mutex_lock(&chopstick_mutex);
		//check if both chopsticks are available if not unlock mutex
		if(chopsticks[*philnum] == 1 && chopsticks[(*philnum + 1) % NUM_CHOPS] == 1){ 
			chopsticks[*philnum] = chopsticks[(*philnum + 1) % NUM_CHOPS] = 0; // take both chops and unlock
			pthread_mutex_unlock(&chopstick_mutex);
			eat(&eattot, *philnum);
			pthread_mutex_lock(&chopstick_mutex); //get lock to return chops
			chopsticks[*philnum] = chopsticks[(*philnum + 1) % NUM_CHOPS] = 1;
			pthread_mutex_unlock(&chopstick_mutex);
			think(&thinktot, *philnum);
		}else{
			pthread_mutex_unlock(&chopstick_mutex);
			sleep(RETRY_WAIT);
		}
	}
}

int main(int argc, char *argv[]){
	int *philnum;
	pthread_t philos[NUM_PHILOS];
	pthread_mutex_init(&chopstick_mutex, NULL);	
	for(int i = 0; i < NUM_CHOPS; i++){
		chopsticks[i] = 1;
	}
	for(int i = 0; i < NUM_PHILOS; i++){
		philnum = (int*)malloc(sizeof(int));
		*philnum = i;
		pthread_create(&philos[i], NULL, (void*) philosophise, (void*) philnum);
	}
	for(int i = 0; i < NUM_PHILOS; i++){
		pthread_join(philos[i], NULL);
	}
	return 0;
}
