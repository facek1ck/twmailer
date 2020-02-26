#define _DEFAULT_SOURCE
#include "serverFunctions.h"
#include <time.h>

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
                char *username = line;
                line = strtok(NULL, "\n");
                char *msgNr = line;
                readMail(client_socket, username, msgNr);
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

    close(client_socket);
    return EXIT_SUCCESS;
}

int ldapLogin(char *line)
{
    // strcpy(username, "if18b101");
    return 1; //TODO: implement LDAP login
}

int saveMail(char *line)
{
    char receiverPath[255], receiverFilePath[255];
    char receiver[10];
    mkdir(path, 0777);
    FILE *fPtr;

    int lineCount = 0;
    while (line)
    {
        if (lineCount == 1) //receiver's username
        {
            strncpy(receiver, line, 10);
            if (strlen(receiver) > 8)
            {
                return 0;
            }
            snprintf(receiverPath, sizeof(receiverPath), "%s/%s", path, receiver);
            mkdir(receiverPath, 0777);
            srand(time(0));
            snprintf(receiverFilePath, sizeof(receiverFilePath), "%s/%d.txt", receiverPath, rand());
            fPtr = fopen(receiverFilePath, "w+");
            if (fPtr == NULL)
            {
                return 0;
            }
            fputs(receiver, fPtr); //put receiverName
            fputc('\n', fPtr);
        }
        else if (lineCount == 2)
        {
            fputs(line, fPtr); //put subject
            fputc('\n', fPtr);
        }
        else if (lineCount > 2)
        {
            fputs(line, fPtr);
            fputc('\n', fPtr);
        }

        line = strtok(NULL, "\n");
        lineCount++;
    }
    if (fPtr != NULL)
    {
        fclose(fPtr);
    }
    return 1;
}

void listMails(int client_socket)
{
    int fileCount = 0;
    //path to username
    char buffer[BUF];

    DIR *dirp;
    struct dirent *entry;
    char userpath[100];

    snprintf(userpath, sizeof(userpath), "%s/%s", path, "if18b101");

    fileCount = getMailCount(userpath);
    dirp = opendir(userpath);
    fileCount = 0;

    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char filePath[256];
            snprintf(filePath, sizeof(filePath) + sizeof(userpath), "%s/%s", userpath, entry->d_name);
            FILE *file = fopen(filePath, "r");
            char line[256];
            int lineCount = 0;
            while (fgets(line, sizeof(line), file))
            {
                if (lineCount == 1)
                {
                    if (fileCount == 0)
                    {
                        sprintf(buffer, "%d - %s", fileCount, line);
                    }
                    else
                    {
                        sprintf(buffer, "%s\n%d - %s", buffer, fileCount, line);
                    }
                    fileCount++;
                    break;
                }
                lineCount++;
            }
            fclose(file);
        }
    }
    send(client_socket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
}
void readMail(int client_socket, char *username, char *msgNr)
{
}

int deleteMail(int client_socket, char *line)
{
    return 0;
}

int getMailCount(char *path)
{
    int fileCount = 0;
    //path to username

    DIR *dirp;
    struct dirent *entry;

    dirp = opendir(path);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            fileCount++;
        }
    }
    return fileCount;
}