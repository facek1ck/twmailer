#ifndef SERVERFUNCTIONS_H
#define SERVERFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define BUF 1024
char *username;
char *path;
int port;
int handleClient(int client_socket);
int ldapLogin(char *line); // set global char *username;
int saveMail(char *line);
void listMails(int client_socket);
void readMail(int client_socket, int msgNr);
int deleteMail(int client_socket, int msgNr);

#endif