CC = gcc -Wall -std=gnu99 -ggdb

all:
	rm -f test_pipe
	$(CC) client.c -o client
	$(CC) server.c -o server
	make tests
	make clean

clean:
	rm -f *.o

tests:
