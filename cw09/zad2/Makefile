CC = gcc -Wall -std=gnu99
LDLIBS = -lpthread -L.. -lproducer_consumer

all:
	$(CC) -c ../producer_consumer.c -o ../producer_consumer.o
	ar rcs ../libproducer_consumer.a ../producer_consumer.o
	$(CC) main.c -o program $(LDLIBS)
	make clean
	make tests

clean:
	rm -f *.o
	rm -f ../*.o

tests:
	./program ../configuration.txt