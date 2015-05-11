
a.out: communication.o test.c
	gcc -g -O0 test.c  communication.o -lpthread

communication.o: communication.c
	gcc -c -g -O0 communication.c 
