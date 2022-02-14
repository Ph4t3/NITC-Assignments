CC=gcc
CFLAGS=-g -lpthread

default: tcpclient.o tcpserver.o dummy

tcpclient.o: tcpclient.c
	$(CC) $(CFLAGS) -o $@ $< 

tcpserver.o: tcpserver.c
	$(CC) $(CFLAGS) -o $@ $< 

dummy:
	seq 10000000 > file.bin
	truncate --size=50M myfile.bin

graph:
	gnuplot plot.plt

clean: 
	rm *.o outfile stats.dat myfile.bin
