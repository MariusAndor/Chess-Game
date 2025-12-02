CC = gcc
CFLAGS = -Wall 

all: server client 

server: server.o
	$(CC) $(CFLAGS) server.o -o server

client: client.o
	$(CC) $(CFLAGS) client.o -o client

server.o: server.c
	$(CC) $(CFLAGS) -c server.c -o server.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c -o client.o

run_both:
	./server & 
	./client

clean:
	rm -f server client server.o client.o
