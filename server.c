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

// determines which server to join
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

int run(char *buff, int length)
{
    int exitStatus = -1;
    // child Process
    if (fork() == 0)
    {
        system(buff);
        fprintf(stderr, "\nrun: after running system");
        exit(1);
    }
    else
    {
        wait(NULL);
    }
}

void ServeClient(int sd, const char *serverType)
{
    char message[MAX_LENGTH];
    int n;
    // make the screen descriptor designate the client socket
    dup2(sd, STDOUT_FILENO);
    dup2(sd, STDIN_FILENO);
    dup2(sd, STDERR_FILENO);
    while (1)
    {
        fprintf(stderr, "\nserveClient: reading from client");
        n = read(sd, message, MAX_LENGTH);

        // quit if the client sends 'quit'
        message[n] = '\0';
        fprintf(stderr, "\nserveClient: client command: %s", message);
        if (strncmp(message, "quit", 4) == 0)
        {
            fprintf(stderr, "Client Quit: %s\n", message);
            close(sd);
            exit(0);
        }
        else
        {
            run(message, n);
            write(sd, "DONE----", 9);
        }
    }
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
                continue;
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
                    continue;
                }
                // fprintf(stderr, "\n Total clients: %d, A: %d, B: %d, join_server: %d", total_clients, no_of_server_a_clients, no_of_server_b_clients, join_server_a);
            }
        }

        else
            fprintf(stderr, "\nClient Accepted");

        if (fork() == 0)
            ServeClient(client, argv[1]);
        else
        {
            // decrease client count
        }
    }
    return 0;
}
