CC=gcc
CCF=-w
CO=-c
all: server client util.o
server: server.c util.o
	$(CC) $(CCF) util.o server.c -o server
client: client.c util.o
	$(CC) $(CCF) util.o client.c -o client
util.o: util.c util.h
	$(CC) $(CCF) $(CO) util.c -o util.o
clean:
	rm -f *.o server client
