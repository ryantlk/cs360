CFLAGS = -Wall -std=c99
LIBS = -lm -lpthread

all: philo

philo: random_r.o philo.o
			gcc $(CFLAGS) philo.o random_r.o -o philo $(LIBS)

random_r.o: random_r.c random_r.h
			gcc -c $(CFLAGS) random_r.c -o random_r.o

philo.o: philo.c
			gcc -c $(CFLAGS) philo.c -o philo.o

.PHONY: clean

clean:
			rm philo.o random_r.o philo
