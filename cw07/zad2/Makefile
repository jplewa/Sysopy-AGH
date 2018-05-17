CC = gcc -std=gnu99
FLAGS = -Wall -L. -lbarber_shop
LDLIBS = -lrt -lpthread

all:
	$(CC) -c barber_shop.c -o barber_shop.o
	ar rcs libbarber_shop.a barber_shop.o
	$(CC) barber.c $(FLAGS) $(LDLIBS) -o barber 
	$(CC) customers.c $(FLAGS) $(LDLIBS) -o customers
	make tests
	make clean

clean:
	rm -f *.o

tests:
