CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f lista.txt
	./program / '<' 2018-01-15 >> lista.txt