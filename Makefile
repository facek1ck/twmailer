all: server client

server: ./server/server.c
<<<<<<< HEAD
		gcc -g -Wall -o out/myserver ./server/server.c

client: ./client/client.c
		gcc -g -Wall -o out/myclient ./client/client.c
	
clean: 
		rm -rf out/*
=======
		gcc -g -Wall -o myserver ./server/server.c

client: ./client/client.c
		gcc -g -Wall -o myclient ./client/client.c
	
clean: 
		sudo rm -rf *.zip myserver myclient *.dSYM
		find . -name "*.o" -type f -delete
>>>>>>> basic 2-way server-client communication
