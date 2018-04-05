CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) ../zad3a/main.c -o program
	$(CC) time_test.c -o time_test
	$(CC) memory_test.c -o memory_test
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f plik
	echo pwd >> plik
	echo time_test >> plik
	echo pwd >> plik
	./program plik 1 320
	rm -f plik
	echo pwd >> plik
	echo memory_test >> plik
	echo pwd >> plik
	./program plik 1 32