#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF 256
int main()
{
    // create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    //check for error with the connection
    if (connection_status == -1)
    {
        printf("\nThere was an error establishing a connection to the remote socket \n\n");
    }

    //recieve data from the server
    char server_response[BUF];
    recv(network_socket, &server_response, BUF - 1, 0);

    // print server response
    printf("The Server sent data %s", server_response);

    //close the socket
    close(network_socket);
    return 0;
}