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

using namespace std;

#define PORT    5555
#define MAXMSG  512
#define GROUP "group1"

char current_message [MAXMSG];
/*
struct client_information
{
    std::string username;
    std::string ip_addr;
};
*/

bool user_authentication()
{

}

string generate_server_id()
{
    FILE *file_pipe;
    string server_id;
    char command[20];
    char data[512];

    // Execute a process listing
    sprintf(command, "fortune -s");

    // Setup our pipe for reading and execute our command.
    file_pipe = popen(command,"r");

    // Error handling

    // Get the data from the process execution
    server_id = fgets(data, 512 , file_pipe);

    // the data is now in 'data'
    if (pclose(file_pipe) != 0)
        fprintf(stderr," Error: Failed to close command stream \n");

    return server_id + GROUP;
}

void command_list (int connection, fd_set &active_fd_set)
{
    if(current_message[0] == 'I' &&
        current_message[1] == 'D')
    {
        generate_server_id();
    }

}

int read_from_client (int fd)
{
    char buffer[MAXMSG];
    int nbytes;

    nbytes = read (fd, buffer, MAXMSG);

    //current_message += buffer;
    //current_message += " ";

    if (nbytes < 0)
    {
        /* Read error. */
        perror ("read");
        exit (EXIT_FAILURE);
    }
    else if (nbytes == 0)
    {
        /* End-of-file. */
        return -1;
    }
    else if(buffer == "LEAVE")
    {
        return 0;
    }

    else
    {
        /* Data read. */
        fprintf (stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}

int make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Give the socket a name. */
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

    /* Create the socket and set it up to accept connections. */
    sock = make_socket (PORT);
    if (listen (sock, 1) < 0)
    {
        perror ("listen");
        exit (EXIT_FAILURE);
    }

    /* Initialize the set of active sockets. */
    FD_ZERO (&active_fd_set);
    FD_SET (sock, &active_fd_set);

    while (1)
    {
      /* Block until input arrives on one or more active sockets. */
      read_fd_set = active_fd_set;
      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
      {
          perror ("select");
          exit (EXIT_FAILURE);
      }


      /* Service all the sockets with input pending. */
      for (i = 0; i < FD_SETSIZE; ++i)
      {
        if (FD_ISSET (i, &read_fd_set))
        {
            if (i == sock)
            {
                /* Connection request on original socket. */
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
                fprintf (stderr, "Er í els-unni\n");

                /* Data arriving on an already-connected socket. */
                if (read_from_client (i) < 0)
                {
                    close (i);
                    FD_CLR (i, &active_fd_set);
                }

                //command_list(connection, &active_fd_set);
            }
          }

      }
    }
}
