#include "serverFunctions.h"

int main(int argc, char *argv[])
{
    port = atoi(argv[1]);
    path = argv[2];

    char buffer[BUF];
    //create the server socket
    int server_socket, client_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t addrlen;
    pid_t pid;

    //definde the server address
    struct sockaddr_in server_address, cliaddress;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to the specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    {
        perror("bind error");
        return EXIT_FAILURE;
    }
    listen(server_socket, 5);

    addrlen = sizeof(struct sockaddr_in);

    while (1)
    {
        printf("Waiting for connections...\n");
        client_socket = accept(server_socket, (struct sockaddr *)&cliaddress, &addrlen);
        if (client_socket > 0)
        {
            printf("Client connected from %s:%d...\n", inet_ntoa(cliaddress.sin_addr), ntohs(cliaddress.sin_port));
            pid = fork();
        }

        if (pid == 0)
        {
            strcpy(buffer, "Welcome to myserver. PID: ");
            char firstLine[20];
            sprintf(firstLine, "%d%c", getpid(), '\n');
            strcat(buffer, firstLine);
            send(client_socket, buffer, strlen(buffer), 0);
            handleClient(client_socket);
            exit(3);
        }
        if (pid > 0)
        {
        }
    }
    close(server_socket);
    return EXIT_SUCCESS;
}