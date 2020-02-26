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
                readMail(client_socket, line);
            }
            else if (strcmp("DEL", line) == 0)
            {
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

    return 1; //TODO: Remove this when in FH Network

    LDAP *ld;                /* LDAP resource handle */
    LDAPMessage *result, *e; /* LDAP result handle */
    BerElement *ber;         /* array of attributes */
    char *attribute;
    char filter[50];
    BerValue **vals;

    BerValue *servercredp;
    BerValue cred;
    cred.bv_val = (char *)BIND_PW;
    cred.bv_len = strlen(BIND_PW);
    int i, rc = 0;

    sprintf(filter, "(uid=%s)", username);

    const char *attribs[] = {"uid", "cn", "sn", "dn", NULL}; /* attribute array for search */

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

    /* perform ldap search */
    rc = ldap_search_ext_s(ld, SEARCHBASE, SCOPE, filter, (char **)attribs, 0, NULL, NULL, NULL, 500, &result);
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }

    printf("\n");

    if (ldap_first_entry(ld, result) != NULL)
    {

        e = ldap_first_entry(ld, result);

        if (ldap_first_entry(ld, result) != NULL)
            printf("DN: %s\n", ldap_get_dn(ld, e));

        /* Now print the attributes and values of each found entry */
        for (attribute = ldap_first_attribute(ld, e, &ber);
             attribute != NULL; attribute = ldap_next_attribute(ld, e, ber))
        {
            if ((vals = ldap_get_values_len(ld, e, attribute)) != NULL)
            {
                for (i = 0; i < ldap_count_values_len(vals); i++)
                {
                    printf("\t%s: %s\n", attribute, vals[i]->bv_val);
                }
                ldap_value_free_len(vals);
            }

            cred.bv_val = (char *)&password;
            cred.bv_len = strlen((char *)&password);
            ldap_sasl_bind_s(ld, BIND_USER, LDAP_SASL_SIMPLE, &cred, NULL, NULL, &servercredp);

            /* free memory used to store the attribute */
            ldap_memfree(attribute);
        }

        /* free memory used to store the value structure */
        if (ber != NULL)
            ber_free(ber, 0);

        printf("\n");
    }
    else
    {
        printf("LDAP login failed\n");
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 0;
    }

    /* free memory used for result */
    ldap_msgfree(result);
    printf("LDAP login suceeded\n");

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

    snprintf(userpath, sizeof(userpath), "%s/%s", path, username);

    fileCount = getMailCount(userpath);
    dirp = opendir(userpath);
    fileCount = 0;

    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *filePath = malloc(500);
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

void readMail(int client_socket, char *line)
{
    char buffer[BUF];
    memset(buffer, 0, sizeof(buffer));

    int lineCount = 0;
    char *msgNr;
    while (line)
    {
        if (lineCount == 1) //msgNr
        {
            msgNr = line;
        }

        line = strtok(NULL, "\n");
        lineCount++;
    }

    char userpath[100];
    char *filepath = malloc(255);
    sprintf(userpath, "%s/%s", path, username);
    filepath = getFilePathByNumber(userpath, msgNr);

    if (filepath != NULL)
    {
        FILE *file = fopen(filepath, "r");
        // FILE *file = fopen(strcat(userpath, message), "r");
        // int text;
        char chunk[128];

        if (file)
        {
            // while ((text = getc(file)) != EOF)
            //     putchar(text);

            // fclose(file);

            // sprintf(buffer, "%s", text);
            // send(client_socket, buffer, strlen(buffer), 0);

            while (fgets(chunk, sizeof(chunk), file) != NULL)
            {
                strcat(buffer, chunk);
            }

            strcat(buffer, "\n");
            send(client_socket, buffer, strlen(buffer), 0);

            fclose(file);
        }
        else
        {
            printf("File not found\n");
            strcpy(buffer, "ERR\n");
            send(client_socket, buffer, strlen(buffer), 0);
        }
    }
    else
    {
        printf("Dir not found\n");
        strcpy(buffer, "ERR\n");
        send(client_socket, buffer, strlen(buffer), 0);
    }
    // checken ob datei gibt und dann printen
}

int deleteMail(int client_socket, char *line)
{
    char buffer[BUF];
    memset(buffer, 0, sizeof(buffer));

    int lineCount = 0;
    char *msgNr;
    while (line)
    {
        if (lineCount == 1) //msgNr
        {
            msgNr = line;
        }

        line = strtok(NULL, "\n");
        lineCount++;
    }

    char userpath[100];
    char *filepath = malloc(255);
    sprintf(userpath, "%s/%s", path, username);
    filepath = getFilePathByNumber(userpath, msgNr);
    if (filepath != NULL)
    {
        if (remove(filepath) == 0)
            printf("file deleted.");
        else
            printf("Delete not successful.");
    }
    else
    {
        printf("Dir or file not found\n");
        strcpy(buffer, "ERR\n");
        send(client_socket, buffer, strlen(buffer), 0);
    }
    return 1;
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

char *getFilePathByNumber(char *userPath, char *msgNr)
{
    int fileCount = 0;
    char *filePath = malloc(500);
    DIR *dirp;
    struct dirent *entry;
    dirp = opendir(userPath);
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            if (fileCount == atoi(msgNr))
            {
                sprintf(filePath, "%s/%s", userPath, entry->d_name);
                return filePath;
            }
            fileCount++;
        }
    }
    return NULL;
}
