all: server client

server: server/server.c server/serverFunctions.c server/serverFunctions.h
		gcc -g -Wall server/server.c server/serverFunctions.c -o out/myserver

client: client/client.c
		gcc -g -Wall -o out/myclient client/client.c
	
clean: 
		rm -rf out/*
