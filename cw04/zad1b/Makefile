CC = gcc -Wall -std=gnu99 -ggdb

all: 
	$(CC) main.c -o program
	make script
	make clean

clean:
	rm -f *.o

script:
	rm -f script
	echo '#!/bin/bash' >> script
	echo >> script
	echo 'while true; do date; sleep 1; done' >> script