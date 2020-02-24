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
            line = strtok(strdup(buffer), "\n");

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

int ldapLogin(char *line)
{
    return 0;
}

int saveMail(char *line)
{
    char *recieverPath;
    char receiver[8];
    char sender[8];
    mkdir(path, 0777);
    FILE *fPtr;

    int lineCount = 0;
    while (line)
    {
        if (lineCount == 1)
        {
            strcpy(sender, line);
        }
        else if (lineCount == 2) //reciever's username
        {
            strcpy(receiver, line);
            if (strlen(receiver) > 8)
            {
                return 0;
            }
            recieverPath = strcat(strcat(path, "/"), receiver);
            fPtr = fopen(strcat(strcat(recieverPath, "/"), strcat(receiver, ".txt")), "a");
            if (fPtr == NULL)
            {
                return 0;
            }
            fputs(sender, fPtr); //put senderName
            fputc('\n', fPtr);
            fputs(receiver, fPtr); //put recieverName
            fputc('\n', fPtr);
        }
        else if (lineCount == 3)
        {
            fputs(line, fPtr); //put subject
            fputc('\n', fPtr);
        }
        else if (lineCount > 3)
        {
            fputs(line, fPtr);
            fputc('\n', fPtr);
        }

        line = strtok(NULL, "\n");
        lineCount++;
    }
    return 0;
}

void listMails(int client_socket)
{
}
void readMail(int client_socket, char *line)
{
    int fileCount;
    //path to username

    DIR *dirp;
    struct dirent *entry;
    char *userpath = strcat(strcat(path, "/"), username);

    dirp = opendir(userpath);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            fileCount++;
        }
    }
    printf("filecount:" + fileCount);
}
int deleteMail(int client_socket, char *line)
{
    return 0;
}