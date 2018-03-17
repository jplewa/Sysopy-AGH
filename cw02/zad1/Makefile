CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make tests
	make clean

clean:
	rm -f *.o

tests:
	rm -f wyniki.txt
	rm -f data_lib
	rm -f data_sys

	echo "1. RECORD SIZE = 4" >> wyniki.txt
	./program generate data_sys 1000 4
	echo "  1.1. 1000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 1000 4 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 1000 4 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 1000 4 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 1000 4 lib >> wyniki.txt

	./program generate data_sys 3000 4
	echo "  1.2. 3000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 3000 4 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 3000 4 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 3000 4 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 3000 4 lib >> wyniki.txt

	echo "" >> wyniki.txt
	echo "2. RECORD SIZE = 512" >> wyniki.txt
	./program generate data_sys 1000 512
	echo "  2.1. 1000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 1000 512 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 1000 512 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 1000 512 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 1000 512 lib >> wyniki.txt

	./program generate data_sys 2000 512
	echo "  1.2. 2000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 2000 512 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 2000 512 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 2000 512 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 2000 512 lib >> wyniki.txt

	echo "" >> wyniki.txt
	echo "3. RECORD SIZE = 4096" >> wyniki.txt
	./program generate data_sys 1000 4096
	echo "  3.1. 1000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 1000 4096 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 1000 4096 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 1000 4096 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 1000 4096 lib >> wyniki.txt

	./program generate data_sys 2000 4096
	echo "  3.2. 2000 records" >> wyniki.txt
	echo -n "       | System copy  |" >> wyniki.txt 
	./program copy data_sys data_lib 2000 4096 sys >> wyniki.txt
	echo -n "       | Library copy |" >> wyniki.txt
	./program copy data_sys data_lib 2000 4096 lib >> wyniki.txt
	echo -n "       | System sort  |" >> wyniki.txt
	./program sort data_sys 2000 4096 sys >> wyniki.txt
	echo -n "       | Library sort |" >> wyniki.txt
	./program sort data_lib 2000 4096 lib >> wyniki.txt