#ifndef SERVERFUNCTIONS_H
#define SERVERFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <ldap.h>
#include <time.h>

#define BUF 1024

#define _DEFAULT_SOURCE

#define LDAP_URI "ldap://ldap.technikum-wien.at:389"
#define SEARCHBASE "dc=technikum-wien,dc=at"
#define SCOPE LDAP_SCOPE_SUBTREE
#define BIND_USER "" /* anonymous bind with user and pw empty */
#define BIND_PW ""

char *username;
char *path;
int port;
int handleClient(int client_socket);
int ldapLogin(char *line); // set global char *username;
int saveMail(char *line);
void listMails(int client_socket);
void readMail(int client_socket, char *line);
int deleteMail(int client_socket, char *line);
int getMailCount(char *path);
char *getFilePathByNumber(char *userPath, char *msgNr);

#endif