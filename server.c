#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int no_of_server_a_clients = 0, no_of_server_b_clients = 0, total_clients = -1;

int join_server_a_or_b()
{
    // a -> 1, b -> 0
    int join_server;
    if (no_of_server_a_clients < MAX_PROCESS_PER_SERVER)
        join_server = 1;
    else if (no_of_server_b_clients < MAX_PROCESS_PER_SERVER)
        join_server = 0;
    else if (total_clients % 2 == 0)
        join_server = 0;
    else
        join_server = 1;

    return join_server;
}

int main(int argc, char const *argv[])
{
    // check if A or B passed for server name
    if (argc < 2)
    {
        printf("\nPlease Pass server A or B\n");
        exit(1);
    }

    int sd, server_a_id, client, n;
    char buffer[MAX_LENGTH];
    if (strcmp(argv[1], "A") == 0)
    {
        createServer(&sd, SERVER_A_PORT_NUMBER);
    }
    else
    {
        createServer(&sd, SERVER_B_PORT_NUMBER);
        connectToServer(&server_a_id, SERVER_A_IP, SERVER_A_PORT_NUMBER);
        fprintf(stderr, "\nServer B connected to Server A");
        write(server_a_id, "s", 1);
    }

    fprintf(stderr, "\n%s Server Started-----------------", argv[1]);

    while (1)
    {
        client = accept(sd, (struct sockaddr *)NULL, NULL);

        // only handle this inside server A
        if (strcmp(argv[1], "A") == 0)
        {
            total_clients++;
            n = read(client, buffer, MAX_LENGTH);
            buffer[n] = '\0';

            // if server B is connected
            if (strncmp(buffer, "s", 1) == 0)
            {
                fprintf(stderr, "\nMessage from Server: %s", buffer);
            }
            else
            {
                fprintf(stderr, "\nMessage from Client: %s", buffer);

                // check condition here which server to join
                int join_server_a = join_server_a_or_b();
                if (join_server_a)
                {
                    no_of_server_a_clients++;
                    write(client, "A", 1);
                }
                else
                {
                    no_of_server_b_clients++;
                    write(client, "B", 1);
                    close(client);
                }
                // fprintf(stderr, "\n Total clients: %d, A: %d, B: %d, join_server: %d", total_clients, no_of_server_a_clients, no_of_server_b_clients, join_server_a);
            }
        }

        else
            fprintf(stderr, "\nClient Accepted");
    }

    return 0;
}
