#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF 256

int main()
{
    char server_message[BUF] = "You have reached the server!";
    //create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //definde the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to the specified IP and port
    bind(server_socket, (struct sockaddr *)&server_socket, BUF - 1);

    listen(server_socket, 20);

    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);

    //send message to client
    send(client_socket, server_message, BUF - 1, 0);

    //close socket
    close(server_socket);

    return 0;
}