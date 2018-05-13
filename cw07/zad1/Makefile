CC = gcc -Wall -std=gnu99 -ggdb
LDLIBS = 

all:
	$(CC) barber.c -o barber $(LDLIBS)
	$(CC) customers.c -o customers $(LDLIBS)
	make tests
	make clean

clean:
	rm -f *.o

tests:
