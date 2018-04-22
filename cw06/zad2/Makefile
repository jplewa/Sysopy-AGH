CC = gcc -Wall -std=gnu99 -ggdb
LDLIBS = -lrt

all:
	rm -f test_pipe
	$(CC) client.c -o client $(LDLIBS)
	$(CC) server.c -o server $(LDLIBS)
	make tests
	make clean

clean:
	rm -f *.o

tests:
