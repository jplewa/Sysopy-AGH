CC = gcc -Wall -std=gnu11 -ggdb

all: static shared zad3a clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o

clean:
	rm -f *.o

zad3_static:
	make static
	$(CC) -o test main.c ./library.a -I .
	make tests

zad3_dynamic:
	make shared
	$(CC) -o test main_dynamic.c -ldl -I .
	make tests

zad3_shared:
	make shared
	$(CC) -o test main.c -L. ./library.so -I .
	make tests

zad3a:
	rm -f raport3a.txt
	echo "----------------------------------------------" >> raport3a.txt
	echo "                STATIC LIBRARY" >> raport3a.txt
	make zad3_static
	echo "----------------------------------------------" >> raport3a.txt
	echo "                SHARED LIBRARY" >> raport3a.txt
	make zad3_shared
	echo "----------------------------------------------" >> raport3a.txt
	echo "     SHARED LIBRARY (DYNAMICALLY LOADED)" >> raport3a.txt
	make zad3_dynamic

tests:
	./test raport3a create_table s 1000 1000 add 50000 remove 50000 remove_and_add 50000 search 1234
	./test raport3a create_table d 1000 1000 add 50000 remove 50000 remove_and_add 50000 search 1234
