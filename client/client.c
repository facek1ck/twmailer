#include "clientFunctions.h"

int main(int argc, char *argv[])
{
    int port = atoi(argv[2]);
    int ip = inet_addr(argv[1]);
    char buffer[BUF];
    int size;
    int menuChoice;

    // create a socket
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = ip;

    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    //check for error with the connection
    if (connection_status == -1)
    {
        printf("\nThere was an error establishing a connection to the remote socket \n\n");
        return EXIT_FAILURE;
    }
    else
    {
        printf("Connection with server (%s) established\n", inet_ntoa(server_address.sin_addr));
        size = recv(network_socket, buffer, BUF - 1, 0);
        buffer[size] = '\0';
        printf("%s", buffer);
    }

    //Login procedure

    char username[10];
    char *password;
    printf("Username:");
    scanf("%s", username);
    password = getpass("Password:"); // hide the user input
    sprintf(buffer, "%s%c%s%c%s%c", "LOGIN", '\n', username, '\n', password, '\n');
    send(network_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    size = recv(network_socket, buffer, BUF - 1, 0);

    if (size > 0)
    {
        buffer[size] = '\0';
        if (!(strcmp(buffer, "OK\n") == 0)) //if the login was not successfull
        {
            printf("%s\n", buffer);
            return 1;
        }
    }
    do
    {
        memset(buffer, 0, sizeof(buffer));
        printMainMenu();
        printf("Your choice:");
        scanf(" %d", &menuChoice);
        while (getchar() != '\n')
            ;
        switch (menuChoice)
        {
        case 1:
        {
            char new_str[BUF];
            strcpy(buffer, "SEND\n");
            int cnt = 0;
            do
            {
                if (cnt < 3)
                {
                    if (cnt == 0)
                    {
                        printf("Receiver:");
                    }
                    else if (cnt == 1)
                    {
                        printf("Subject:");
                    }
                    else if (cnt == 2)
                    {
                        printf("Text:");
                    }
                }
                cnt++;
                fgets(new_str, BUF - 1, stdin);
                strcat(buffer, new_str);
            } while (strcmp(new_str, ".\n") != 0);
            send(network_socket, buffer, strlen(buffer), 0);
            break;
        }
        case 2:
        {
            strcpy(buffer, "LIST\n");
            send(network_socket, buffer, strlen(buffer), 0);
            break;
        }
        case 3:
        {
            char new_str[10];
            int msgNr = 0;
            strcpy(buffer, "READ\n");
            printf("Enter message number: ");
            scanf(" %d", &msgNr);
            while (getchar() != '\n')
                ;
            sprintf(new_str, "%d", msgNr);
            strcat(buffer, new_str);
            send(network_socket, buffer, strlen(buffer), 0);
            break;
        }
        case 4:
        {
            char new_str[10];
            int msgNr = 0;
            strcpy(buffer, "DEL\n");
            printf("Enter message number: ");
            scanf(" %d", &msgNr);
            while (getchar() != '\n')
                ;
            sprintf(new_str, "%d", msgNr);
            strcat(buffer, new_str);
            send(network_socket, buffer, strlen(buffer), 0);
            break;
        }
        case 5:
        {
            close(network_socket);
            signalH(EXIT_SUCCESS);
            break;
        }
        default:
        {
            printf("Wrong menu entry.");
            continue;
        }
        }
        size = recv(network_socket, buffer, BUF - 1, 0);
        if (size > 0)
        {
            buffer[size] = '\0';
            printf("%s", buffer);
        }
        memset(buffer, 0, sizeof(buffer));
        fflush(stdout);
        fflush(stdin);
    } while (strcmp(buffer, "quit\n") != 0);
    close(network_socket);
    return EXIT_SUCCESS;
}