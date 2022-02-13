CC=gcc
CFLAGS=-g -lpthread

default: tcpclient.o tcpserver.o

tcpclient.o: tcpclient.c
	$(CC) $(CFLAGS) -o $@ $< 

tcpserver.o: tcpserver.c
	$(CC) $(CFLAGS) -o $@ $< 

graph:
	gnuplot plot.plt

clean: 
	rm *.o outfile stats.dat
