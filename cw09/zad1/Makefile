CC = gcc -Wall -std=gnu99
LDLIBS = -lpthread

all:
	$(CC) main.c -o program $(LDLIBS)
	make tests
	make clean

clean:
	rm -f *.o


tests:
	./program ../args

