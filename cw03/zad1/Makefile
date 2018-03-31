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
	./program /home/julia/ '<' 2018-03-28 nftw | wc
	./program /home/julia/ '<' 2018-03-28 | wc
