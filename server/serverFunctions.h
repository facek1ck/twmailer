#ifndef SERVERFUNCTIONS.H
#define SERVERFUNCTIONS .H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define BUF 1024

int handleClient(int client_socket);
int ldapLogin(char *line);
int saveMail(char *text);
void listMails(int client_socket);
void readMail(int client_socket, int msgNr);
int deleteMail(int client_socket, int msgNr);

#endif