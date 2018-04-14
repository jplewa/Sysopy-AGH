CC = gcc -Wall -std=gnu99 -ggdb

all: static zad2 clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

clean:
	rm -f *.o

zad2:
	make static
	$(CC) -o test main.c ./library.a -I . -O$(Olevel)
	make tests

tests:
	rm -f raport2.txt	
	./test raport2 create_table d 1000 1000 add 50000 remove 50000 remove_and_add 10000 search 7690
