all: server client

server: server/server.c server/serverFunctions.c server/serverFunctions.h
		gcc -g -Wall -o out/myserver server/server.c

client: client/client.c
		gcc -g -Wall -o out/myclient client/client.c
	
clean: 
		rm -rf out/*
