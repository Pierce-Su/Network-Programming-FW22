 CC = gcc
 LINK_PTHREAD = -lpthread

CLIENT = client.c
SERVER = server.c

CLI_EXE = client
SER_EXE = server

all: server client

server: $(SERVER)
	$(CC) $(SERVER) -o $(SER_EXE) $(LINK_PTHREAD)

client: $(CLIENT)
	$(CC) $(CLIENT) -o $(CLI_EXE) $(LINK_PTHREAD)
