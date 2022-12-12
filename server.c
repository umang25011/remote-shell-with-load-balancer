#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>

void *shared_count_a, *shared_count_b;

void update_shared_count(void *shared_var, int value)
{
    char string_num[5];
    sprintf(string_num, "%d", value);
    memcpy(shared_var, string_num, 10);
}

void *create_shared_memory(size_t size)
{
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(NULL, size, protection, visibility, -1, 0);
}

// determines which server to join
int join_server_a_or_b()
{
    // a -> 1, b -> 0
    int join_server;
    int no_of_server_a_clients = atoi(shared_count_a);
    int no_of_server_b_clients = atoi(shared_count_b);
    // printf("\nint A: %d, int B: %d, shared A: %s, shared B: %s", no_of_server_a_clients, no_of_server_b_clients, shared_count_a, shared_count_b);
    int total_clients = no_of_server_a_clients + no_of_server_b_clients;

    if (no_of_server_a_clients < MAX_PROCESS_PER_SERVER)
        join_server = 1;
    else if (no_of_server_b_clients < MAX_PROCESS_PER_SERVER)
        join_server = 0;
    else if (total_clients % 2 == 0)
        join_server = 1;
    else
        join_server = 0;

    return join_server;
}

int run(char *buff, int length)
{
    int exitStatus = -1;
    // child Process
    if (fork() == 0)
    {
        system(buff);
        // fprintf(stderr, "\nrun: after running system");
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
    int stdout_file = dup(STDOUT_FILENO);

    // dup2(sd, STDOUT_FILENO);
    dup2(sd, STDIN_FILENO);
    dup2(sd, STDERR_FILENO);
    while (1)
    {
        // fprintf(stderr, "\nserveClient: reading from client");
        n = read(sd, message, MAX_LENGTH);

        // quit if the client sends 'quit'
        message[n] = '\0';
        // fprintf(stderr, "\nserveClient: client command: %s", message);
        if (strncmp(message, "quit", 4) == 0)
        {
            fprintf(stderr, "Client Quit: %s\n", message);
            close(sd);
            dup2(stdout_file, STDOUT_FILENO);
            break;
        }
        else
        {
            run(message, n);
            write(sd, "DONE----", 9);
        }
    }
}

void display_clients(const char *server_type)
{
    if (strcmp(server_type, "A") == 0)
    {
        printf("\n-------Client Count-------");
        printf("\n A: %d, B: %d", atoi(shared_count_a), atoi(shared_count_b));
        printf("\n--------------------------\n");
    }
}

int main(int argc, char const *argv[])
{
    shared_count_a = create_shared_memory(10);
    shared_count_b = create_shared_memory(10);

    update_shared_count(shared_count_a, 0);
    update_shared_count(shared_count_b, 0);

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
            n = read(client, buffer, MAX_LENGTH);
            buffer[n] = '\0';

            // if server B is connected
            if (strncmp(buffer, "s", 1) == 0)
            {
                if (fork() == 0)
                {
                    fprintf(stderr, "\nServer B Connected\n");
                    int n;
                    char buffer[11];
                    while (1)
                    {
                        n = read(client, buffer, 10);

                        // quit if the client sends 'quit'
                        buffer[n] = '\0';
                        if (strlen(buffer) == 0)
                        {
                            fprintf(stderr, "\nServer B Disconnected\n");
                            update_shared_count(shared_count_b, 0);
                            exit(0);
                        }
                        else
                        {
                            fprintf(stderr, "\nServer B-> Client Disconnected");
                            update_shared_count(shared_count_b, atoi(shared_count_b) - 1);
                            display_clients(argv[1]);
                        }
                    }
                    exit(0);
                }

                continue;
            }
            else
            {
                // fprintf(stderr, "\nClient Accepted");

                // check condition here which server to join
                int join_server_a = join_server_a_or_b();

                if (join_server_a)
                {
                    write(client, "A", 1);
                }
                else
                {
                    update_shared_count(shared_count_b, atoi(shared_count_b) + 1);
                    display_clients(argv[1]);
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
        {
            // increase process count
            update_shared_count(shared_count_a, atoi(shared_count_a) + 1);
            display_clients(argv[1]);
            ServeClient(client, argv[1]);
            update_shared_count(shared_count_a, atoi(shared_count_a) - 1);

            // if server B, notify server A about client disconnect
            if (strcmp(argv[1], "B") == 0)
            {
                write(server_a_id, shared_count_b, 10);
            }
            else
            {
                printf("\nServer A-> Client Disconnected");
                display_clients(argv[1]);
            }
            exit(0);
        }
    }
    return 0;
}
