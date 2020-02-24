#include "serverFunctions.h"

int main(int argc, char *argv[])
{
    port = atoi(argv[1]);
    path = argv[2];

    char server_message[BUF] = "You have reached the server!";
    //create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //definde the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to the specified IP and port
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    listen(server_socket, 5);

    //accept a connection from client
    int client_socket;
    client_socket = accept(server_socket, NULL, NULL); //replace NULL with structs

    //send message to client
    send(client_socket, server_message, BUF - 1, 0);

    //close socket
    close(server_socket);

    return 0;
}