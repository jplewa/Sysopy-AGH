CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f plik
	echo ls -la >> plik
	echo pwd >> plik
	echo cd / >> plik
	echo ls -la >> plik
	./program plik 1 30