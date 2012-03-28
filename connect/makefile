CFLAGS =  -Wall -std=gnu99

all: connect

connect: connect.o
			gcc $(CFLAGS) connect.c -o connect

connect.o: connect.c
			gcc -c $(CFLAGS) connect.c -o connect.o

.PHONY: clean

clean:
			rm connect.o connect
