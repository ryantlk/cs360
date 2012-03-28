CFLAGS = -Wall -std=gnu99
LIBS = -lm

all: philo

philo: random.o philo.o
			gcc $(CFLAGS) philo.o random.o -o philo $(LIBS)

random.o: random.c random.h
			gcc -c $(CFLAGS) random.c -o random.o

philo.o: philo.c
			gcc -c $(CFLAGS) philo.c -o philo.o

.PHONY: clean

clean:
			rm philo.o random.o philo
