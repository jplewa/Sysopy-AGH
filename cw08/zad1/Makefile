CC = gcc -Wall -std=gnu99
LDLIBS = -lpthread -lm

all:
	$(CC) filter_image.c -o filter_image $(LDLIBS)
	make clean
	make tests

clean:
	rm -f *.o

tests:
	echo "WxH			c		threads		real time" > Times.txt
	for i in 1 2 3 4 5 6 7 8 9 10 ; do \
		for number in 1 2 4 8 ; do \
			./filter_image $${number} ./images/in.pgm ./filters/filter$${i} ./images/out$${i}.pgm >> Times.txt ; \
		done ; \
	done