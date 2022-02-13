CC=gcc

default: tcpclient.o tcpserver.o

tcpclient.o: tcpclient.c
	$(CC) -o $@ $< 

tcpserver.o: tcpserver.c
	$(CC) -o $@ $< 

clean: 
	rm *.o
