#include "serverFunctions.h"

int handleClient(int client_socket)
{
    int size;
    char buffer[BUF];
    char *line = malloc(BUF);

    do
    {
        memset(buffer, 0, sizeof(buffer));
        size = recv(client_socket, buffer, BUF - 1, 0);

        if (size > 0)
        {
            buffer[size] = '\0';
            line = strtok(strdup(buffer), '\n');

            if (strcasecmp("LOGIN", line) == 0)
            {
                if (ldapLogin(line) == 1)
                {
                    strcpy(buffer, "OK\n");
                }
                else
                {
                    strcpy(buffer, "ERR\n");
                }
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp("SEND", line) == 0)
            {
                if (saveMail(line) == 1)
                {
                    strcpy(buffer, "OK\n");
                }
                else
                {
                    strcpy(buffer, "ERR\n");
                }
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp("LIST", line) == 0)
            {
                listMails(client_socket);
            }
            else if (strcmp("READ", line) == 0)
            {
                line = strtok(NULL, "\n");
                readMail(client_socket, line);
            }
            else if (strcmp("DEL", line) == 0)
            {
                line = strtok(NULL, "\n");
                if (deleteMail(client_socket, line) == 1)
                {
                    strcpy(buffer, "OK\n");
                }
                else
                {
                    strcpy(buffer, "ERR\n");
                }
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }
        else if (size == 0)
        {
            printf("Client closed remote socket\n");
            break;
        }
        else
        {
            perror("recv error");
            return EXIT_FAILURE;
        }
        fflush(stdout);
        fflush(stdin);
    } while (strncmp(buffer, "quit", 4) != 0);

    printf("Client closed remote socket\n");
    close(client_socket);
    return EXIT_SUCCESS;
}