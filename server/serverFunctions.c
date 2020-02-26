
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

    return 1;

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