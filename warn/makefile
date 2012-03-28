CFLAGS =  -Wall -std=gnu99

all: warn

warn: warn.o
			gcc $(CFLAGS) warn.c -o warn

warn.o: warn.c
			gcc -c $(CFLAGS) warn.c -o warn.o

.PHONY: clean

clean:
			rm warn.o warn
