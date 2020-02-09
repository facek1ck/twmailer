all: server client

server: ./server/server.c
		gcc -g -Wall -o myserver ./server/server.c

client: ./client/client.c
		gcc -g -Wall -o myclient ./client/client.c
	
clean: 
		sudo rm -rf *.zip myserver myclient *.dSYM
		find . -name "*.o" -type f -delete