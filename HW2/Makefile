CC = gcc
LINK_PTHREAD = -lpthread

SERVER = server.c

SER_EXE = server

all: server

server: $(SERVER)
	$(CC) $(SERVER) -o ./build/$(SER_EXE) $(LINK_PTHREAD)
