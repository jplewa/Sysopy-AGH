CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f dane
	rm -f dane2
	rm -f dane3
	./program generate dane 1000 512
	./program copy dane dane2 1000 500 lib
	./program copy dane dane3 1000 500 sys

