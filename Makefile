CC = gcc
CFLAGS = -Wall
LIBS = -lncursesw

SERVER_OBJS = server.o GUI.o chessLogic.o
CLIENT_OBJS = client.o GUI.o chessLogic.o

all: server client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o server $(LIBS)

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o client $(LIBS)

server.o: server.c GUI.h chessLogic.h
	$(CC) $(CFLAGS) -c server.c

client.o: client.c GUI.h chessLogic.h
	$(CC) $(CFLAGS) -c client.c

GUI.o: GUI.c GUI.h
	$(CC) $(CFLAGS) -c GUI.c

chessLogic.o: chessLogic.c chessLogic.h
	$(CC) $(CFLAGS) -c chessLogic.c

clean:
	rm -f *.o server client
