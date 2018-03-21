CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f lista.txt
	rm -f lista2.txt
	./program / '<' 2017-01-15 >> lista.txt
	./program / '<' 2017-01-15 nftw >> lista2.txt