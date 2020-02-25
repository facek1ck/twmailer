#include "clientFunctions.h"

void printMainMenu()
{
    printf("Menu:\n");
    printf("(1)\t SEND\n");
    printf("(2)\t LIST\n");
    printf("(3)\t READ\n");
    printf("(4)\t DEL\n");
    printf("(5)\t QUIT\n");
}

void signalH(int sig)
{
    printf("Closing client\n");
    fflush(stdout);
    kill(getpid(), SIGTERM);
}

void print_usage()
{
    printf("Usage: [-a ServerAdress] [-p PORT]\n\n");
    return;
}