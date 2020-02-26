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
    int lineCount = 0;
    char *password;
    while (line)
    {
        if (lineCount == 1) //receiver's username
        {
            username = line;
        }
        else
        {
            password = line;
        }

        line = strtok(NULL, "\n");
        lineCount++;
    }

    LDAP *ld;                /* LDAP resource handle */
    LDAPMessage *result, *e; /* LDAP result handle */
    BerElement *ber;         /* array of attributes */
    char *attribute;
    char filter[50];
    BerValue **vals;

    BerValue *servercredp;
    BerValue cred;
    cred.bv_val = BIND_PW;
    cred.bv_len = strlen(BIND_PW);
    int i, rc = 0;

    strcpy(filter, "uid=");
    strcat(filter, username);
    printf("Try login for: %s\n", filter);

    const char *attribs[] = {"uid", "cn", NULL}; /* attribute array for search */

    int ldapversion = LDAP_VERSION3;

    /* setup LDAP connection */
    if (ldap_initialize(&ld, LDAP_URI) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_init failed");
        return 0;
    }

    printf("connected to LDAP server %s\n", LDAP_URI);

    if ((rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapversion)) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_set_option(PROTOCOL_VERSION): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }

    if ((rc = ldap_start_tls_s(ld, NULL, NULL)) != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_start_tls_s(): %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }

    /* anonymous bind */
    rc = ldap_sasl_bind_s(ld, BIND_USER, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);

    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "LDAP bind error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }
    else
    {
        printf("bind successful\n");
    }

    /* perform ldap search */
    rc = ldap_search_ext_s(ld, SEARCHBASE, SCOPE, filter, (char **)attribs, 0, NULL, NULL, NULL, 500, &result);

    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }

    if (ldap_count_entries(ld, result) != 1)
    {
        return 0;
    }
    //Takes the result makes new binding with the user's name and password
    if (ldap_first_entry(ld, result) != NULL)
    {
        e = ldap_first_entry(ld, result);
        strcpy(filter, ldap_get_dn(ld, e));
        printf("FILTER: %s\n", filter);

        cred.bv_val = password;
        cred.bv_len = strlen(password);
        rc = ldap_sasl_bind_s(ld, filter, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);

        if (rc != LDAP_SUCCESS)
        {
            fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
            ldap_unbind_ext_s(ld, NULL, NULL);
            return 0;
        }
    }
    ldap_value_free_len(vals);
    ldap_memfree(attribute);
    ldap_msgfree(result);

    ldap_unbind_ext_s(ld, NULL, NULL);
    return 1;
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
                printf("here\n");
                return 0;
            }
            snprintf(receiverPath, sizeof(receiverPath), "%s/%s", path, receiver);
            mkdir(receiverPath, 0777);
            printf("folder created.\n");
            snprintf(receiverFilePath, sizeof(receiverFilePath), "%s/%d.txt", receiverPath, rand());
            fPtr = fopen(receiverFilePath, "w+");
            if (fPtr == NULL)
            {
                printf("here2\n");
                return 0;
            }
            fputs(receiver, fPtr); //put receiverName
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