all: server client

server: server/server.c server/serverFunctions.c server/serverFunctions.h
		gcc -g -Wall server/server.c server/serverFunctions.c -o out/myserver

client: client/client.c client/clientFunctions.c client/clientFunctions.h
		gcc -g -Wall client/client.c client/clientFunctions.c -o out/myclient
	
clean: 
		rm -rf out/*
