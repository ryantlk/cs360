CFLAGS = -Wall -std=c99
INCLUDES = -I$(GET_WORD)/include
LFLAGS = -L$(GET_WORD)/lib
LIBS = -lget

all: frequency

frequency: wordcount.o hashTable.o
			gcc $(CFLAGS) $(INCLUDES) wordcount.o hashTable.o -o frequency $(LFLAGS) $(LIBS)

wordcount.o: wordcount.c
			gcc -c $(CFLAGS) $(INCLUDES) wordcount.c -o wordcount.o

hashTable.o: hashTable.c hashTable.h
			gcc -c $(CFLAGS) $(INCLUDES) hashTable.c -o hashTable.o

.PHONY: clean

clean:
			rm hashTable.o wordcount.o frequency
