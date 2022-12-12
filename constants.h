#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int SERVER_A_PORT_NUMBER = 5000,
    SERVER_B_PORT_NUMBER = 6000,
    LOAD_BALANCER_PORT_NUMBER = 9656,
    MAX_PROCESS_PER_SERVER = 5,
    MAX_PROCESSES = 1000,
    MAX_WORD_LENGTH = 40,
    MAX_LENGTH = 100000;

char SERVER_A_IP[] = "127.0.1.1";
char SERVER_B_IP[] = "127.0.1.1";
char LOAD_BALANCER_IP[] = "127.0.0.1";

void connectToServer(int *server, char *ip, int portNumber)
{

    socklen_t len;
    struct sockaddr_in servAdd;

    if ((*server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Cannot create a socket\n");
        exit(1);
    }

    // config the server socket
    servAdd.sin_family = AF_INET;
    servAdd.sin_port = htons((uint16_t)portNumber);
    if (inet_pton(AF_INET, ip, &servAdd.sin_addr) < 0)
    {
        fprintf(stderr, " inet_pton() has failed\n");
        exit(2);
    }

    if (connect(*server, (struct sockaddr *)&servAdd, sizeof(servAdd)) < 0)
    {
        fprintf(stderr, "Connection refused...\n");
        fprintf(stderr, "Check if server is running on the right port and try again...\n");
        exit(3);
    }
    // get the port number
}

int createServer(int *server, int portNumber)
{
    socklen_t len;
    struct sockaddr_in servAdd;

    if ((*server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating new socket");
        exit(1);
    }

    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons((uint16_t)portNumber);

    bind(*server, (struct sockaddr *)&servAdd, sizeof(servAdd));
    if (listen(*server, MAX_PROCESSES) < 0)
    {
        printf("Error Creating Server\n");
        exit(1);
    }
}