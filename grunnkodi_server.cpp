#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <array>

using namespace std;

#define PORT    5555
#define MAXMSG  512
#define GROUP "group1"

string current_message;

string generate_server_id()
{

    FILE *file_pipe;
    array<char, 128> input;
    // Execute a process listing
    // Get the data from the process execution
    string fortune = "fortune -s";
    // Setup our pipe for reading and execute our command.
    file_pipe = popen(fortune.c_str(), "r");

    // Error handling

    // the data is now in 'data'
    if (!file_pipe)
    {
      fprintf(stderr," Error: Failed to close command stream \n");
      return 0;
    }

    while(fgets(input.data(), 128 , file_pipe) != NULL)
    {
      printf("Reading");
      return input.data();
    }
}

void command_list (int fd, char buffer [])
{
    char input[MAXMSG];
    strcpy(input, buffer);
    //Command for generating an ID
    if(input[0] == 'I' && input[1] == 'D')
    {
        generate_server_id();
    }
    //Command to leave the server
    else if(buffer[0] == 'L' && buffer[1] == 'E' && buffer[2] == 'A' &&
            buffer[3] == 'V' && buffer[4] == 'E')
    {
        shutdown(fd, SHUT_RDWR);
    }
}

int read_from_client (int fd)
{
    char buffer[MAXMSG];
    int nbytes;

    nbytes = read (fd, buffer, MAXMSG);

    command_list(fd, buffer);

    if (nbytes < 0)
    {
        //Read error.
        perror ("read");
        exit (EXIT_FAILURE);
    }
    else if (nbytes == 0)
    {
        //End-of-file.
        return -1;
    }
    else
    {
        //Data read.
        fprintf (stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}

int make_socket (uint16_t port)
{
    int sock;
    struct sockaddr_in name;

    //Create the socket.
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
          perror ("socket");
          exit (EXIT_FAILURE);
    }

    //Give the socket a name.
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    {
        perror ("bind");
        exit (EXIT_FAILURE);
    }

    return sock;
}

int main (void)
{
    extern int make_socket (uint16_t port);
    int sock;
    fd_set active_fd_set, read_fd_set;
    int i;
    struct sockaddr_in clientname;
    socklen_t size;

    //Create the socket and set it up to accept connections.
    sock = make_socket (PORT);
    if (listen (sock, 1) < 0)
    {
        perror ("listen");
        exit (EXIT_FAILURE);
    }

    //Initialize the set of active sockets.
    FD_ZERO (&active_fd_set);
    FD_SET (sock, &active_fd_set);

    while (1)
    {
      //Block until input arrives on one or more active sockets.
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
      {
          perror ("select");
          exit (EXIT_FAILURE);
      }


      //Service all the sockets with input pending.
      for (i = 0; i < FD_SETSIZE; ++i)
      {
        if (FD_ISSET (i, &read_fd_set))
        {
            if (i == sock)
            {
                //Connection request on original socket.
                int connection;
                size = sizeof (clientname);
                connection = accept (sock,(struct sockaddr *) &clientname, &size);
                if (connection < 0)
                {
                    perror ("accept");
                    exit (EXIT_FAILURE);
                }
                fprintf (stderr,
                         "Server: connect from host %s, port %hd.\n",
                         inet_ntoa (clientname.sin_addr),
                         ntohs (clientname.sin_port));
                FD_SET (connection, &active_fd_set);
            }
            else
            {
                //Data arriving on an already-connected socket.
                if (read_from_client (i) < 0)
                {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                }
            }
          }

      }
    }
}
