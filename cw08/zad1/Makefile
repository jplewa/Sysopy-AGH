CC = gcc -Wall -std=gnu99 -ggdb
LDLIBS = -lpthread -lm

all:
	rm -f test_pipe
	$(CC) filter_image.c -o filter_image $(LDLIBS)
	make tests
	make clean

clean:
	rm -f *.o

tests:
