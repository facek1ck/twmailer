#define _DEFAULT_SOURCE
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
                    printf("message saved");
                    strcpy(buffer, "OK\n");
                }
                else
                {
                    strcpy(buffer, "ERR\n");
                }
                printf("\nbuffer before sending: %s", buffer);
                send(client_socket, buffer, strlen(buffer), 0);
            }
            else if (strcmp("LIST", line) == 0)
            {
                line = strtok(NULL, "\n");
                listMails(client_socket, line);
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

    printf("Client closed remote socket\n");
    close(client_socket);
    return EXIT_SUCCESS;
}

int ldapLogin(char *line)
{
    return 1; //TODO: implement LDAP login
}

int saveMail(char *line)
{
    char *recieverPath;
    char receiver[10];
    mkdir(path, 0777);
    FILE *fPtr;

    int lineCount = 0;
    while (line)
    {
        if (lineCount == 1) //reciever's username
        {
            strncpy(receiver, line, 10);
            if (strlen(receiver) > 8)
            {
                printf("here\n");
                return 0;
            }
            recieverPath = strcat(strcat(path, "/"), receiver);
            mkdir(recieverPath, 0777);
            printf("folder created.\n");
            fPtr = fopen(strcat(strcat(recieverPath, "/"), strcat(receiver, ".txt")), "w+");
            if (fPtr == NULL)
            {
                printf("here2\n");
                return 0;
            }
            fputs(receiver, fPtr); //put recieverName
            fputc('\n', fPtr);
            printf("put receiver\n");
        }
        else if (lineCount == 2)
        {
            fputs(line, fPtr); //put subject
            fputc('\n', fPtr);
            printf("put subject");
        }
        else if (lineCount > 2)
        {
            fputs(line, fPtr);
            fputc('\n', fPtr);
            printf("put line");
        }
        
        line = strtok(NULL, "\n");
        lineCount++;
    }
    printf("message finished");
    if (fPtr != NULL)
    {
        fclose(fPtr);
    }
    return 1;
}

void listMails(int client_socket, char *username)
{
    int fileCount = 1;
    //path to username

    DIR *dirp;
    struct dirent *entry;
    char *userpath = strcat(strcat(path, "/"), username);

    fileCount = getMailCount(userpath);
    printf("File count: %d ", fileCount);
    printf("Overview: \n");
    dirp = opendir(userpath);
    fileCount = 0;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            if (strstr(entry->d_name, ".txt"))
            {
                FILE *file = fopen(entry->d_name, "r");
                char line[256];
                int lineCount = 0;
                while (fgets(line, sizeof(line), file))
                {
                    if (lineCount == 2)
                    {
                        printf("%d - %s", fileCount, line);
                        fileCount++;
                        break;
                    }
                    lineCount++;
                }
            }
        }
    }
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