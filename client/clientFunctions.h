#ifndef CLIENTFUNCTIONS_H
#define CLIENTFUNCTIONS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUF 1024

void printMainMenu();

void signalH(int sig);

void print_usage();

#endif