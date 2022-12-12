#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    char buffer[MAX_LENGTH];
    int server, n;

    connectToServer(&server, SERVER_A_IP, SERVER_A_PORT_NUMBER);
    write(server, "c", 1);

    n = read(server, buffer, 1);
    buffer[n] = '\0';

    if (strncmp(buffer, "A", 1) == 0)
    {
        fprintf(stderr, "\nConnected To Server %s", buffer);
    }
    else
    {
        connectToServer(&server, SERVER_B_IP, SERVER_B_PORT_NUMBER);
        fprintf(stderr, "\nConnected To Server %s", buffer);
    }

    while (1)
    {
        // prompt
        write(STDOUT_FILENO, "--------------------------\n\n\n>>>", 32);
        n = read(STDIN_FILENO, buffer, MAX_LENGTH);
        buffer[n] = '\0';

        
        write(server, buffer, strlen(buffer) + 1);
        // user types 'quit' to close the connection
        if (strncmp(buffer, "quit", 4) == 0)
        {
            printf("Connection Ended\n");
            close(server);
            exit(0);
        }
        do
        {
            if (n = read(server, buffer, MAX_LENGTH))
            {
                buffer[n] = '\0';
                write(STDOUT_FILENO, buffer, n + 1);
                // fprintf(stderr, "\nOutput From Server: %d", n);
            }
        } while (strncmp(buffer, "DONE----", 8));
        // strncmp(buffer, "----------------------------------------", 40)
    }

    return 0;
}
