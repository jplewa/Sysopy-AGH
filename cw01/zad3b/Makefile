CC = gcc -Wall -std=gnu11 -ggdb
Olevel = 1

all: static shared zad3b clean

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
	$(CC) -o test main.c ./library.a -I . -O$(Olevel)
	make tests

zad3_dynamic:
	make shared
	$(CC) -o test main_dynamic.c -ldl -I . -O$(Olevel)
	make tests

zad3_shared:
	make shared
	$(CC) -o test main.c -L. ./library.so -I . -O$(Olevel)
	make tests

zad3a: 
	echo "----------------------------------------------" >> raport3b.txt
	echo "                STATIC LIBRARY" >> raport3b.txt
	make zad3_static
	echo "----------------------------------------------" >> raport3b.txt
	echo "                SHARED LIBRARY" >> raport3b.txt
	make zad3_shared
	echo "----------------------------------------------" >> raport3b.txt
	echo "     SHARED LIBRARY (DYNAMICALLY LOADED)" >> raport3b.txt
	make zad3_dynamic

zad3b:
	rm -f raport3b.txt
	echo "----------------------------------------------" >> raport3b.txt
	echo "*********** OPTIMIZATION LEVEL: 0 ************" >> raport3b.txt
	$(MAKE) Olevel=0 zad3a
	echo "----------------------------------------------" >> raport3b.txt
	echo "*********** OPTIMIZATION LEVEL: 1 ************" >> raport3b.txt
	$(MAKE) Olevel=1 zad3a
	echo "----------------------------------------------" >> raport3b.txt
	echo "*********** OPTIMIZATION LEVEL: 2 ************" >> raport3b.txt
	$(MAKE) Olevel=2 zad3a
	echo "----------------------------------------------" >> raport3b.txt
	echo "*********** OPTIMIZATION LEVEL: 3 ************" >> raport3b.txt
	$(MAKE) Olevel=3 zad3a
	echo "----------------------------------------------" >> raport3b.txt
	echo "*********** OPTIMIZATION LEVEL: s ************" >> raport3b.txt
	$(MAKE) Olevel=s zad3a
	
tests:
	./test raport3b create_table s 1000 1000 add 50000 remove 50000 remove_and_add 50000 search 1234
	./test raport3b create_table d 1000 1000 add 50000 remove 50000 remove_and_add 50000 search 1234
