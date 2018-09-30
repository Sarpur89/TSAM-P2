#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	struct sockaddr_in serv; // Main socket variable.
	int fd; // Socket file descriptor
	char server_response[512] = ""; // Store messages sent by the server.

	fd = socket(AF_INET, SOCK_STREAM, 0);
	serv.sin_family = AF_INET;
	serv.sin_port = htons(5555);

	inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr); // Bind the client to localhost
	connect(fd, (struct sockaddr *)&serv, sizeof(serv)); //Connects the client to the server.
	while(1)
	{
	    printf("Enter a message: ");
	    fgets(server_response, 512, stdin);
	    send(fd, server_response, strlen(server_response), 0);
	}

	return 0;
}
