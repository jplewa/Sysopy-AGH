CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	$(CC) test_io.c -o test_io
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f plik
	echo "./test_io f Ala | ./test_io p ma | ./test_io p kota | ./test_io p i | ./test_io l rybki" >> plik
	echo "ls -la | grep main | wc" >> plik
	echo "ls / -la | grep a | wc | cat | cat" >> plik
	echo "ls -la" >> plik
	echo "./test_io f Ala | ./test_io p ma | cat | cat | cat | cat | cat | cat | cat | cat | cat" >> plik
	echo "./test_io f Ala ma kota i rybki i zolwia i psa i chomika" >> plik
	./program plik