CC = gcc -Wall -std=gnu99
LDLIBS = -lpthread

all:
	rm -f test_pipe
	$(CC) main.c -o program $(LDLIBS)
	make tests
	make clean

clean:
	rm -f *.o

tests:
	./program ../args