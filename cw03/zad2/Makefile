CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	./program plik