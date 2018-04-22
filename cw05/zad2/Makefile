CC = gcc -Wall -std=gnu99 -ggdb

all:
	rm -f test_pipe
	$(CC) master.c -o master
	$(CC) slave.c -o slave
	make tests
	make clean

clean:
	rm -f *.o

tests:
	./master test_pipe