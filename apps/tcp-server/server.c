#include <stdio.h>
#include "server.h"

typedef struct sockaddr common_addr;
typedef struct sockaddr_in myinet_addr;

int main(int argc, char *argv[])
{
	int socket_fd, connet_fd;
	myinet_addr server_addr, client_addr;
	char buf[MAXBUF] = "";
	socklen_t addrlen;

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(-1);
	}
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(8000);
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.17");
	
	if (bind(socket_fd, (common_addr*)(&server_addr), sizeof(server_addr)) == -1) 
	{
		perror("bind");
		exit(-1);
	}

	if (listen(socket_fd, 10) == -1) 
	{
		perror("listen");
		exit(-1);
	}
	int i;
	while(1)
	{
		if ((connet_fd = accept(socket_fd, (common_addr*)&client_addr, &addrlen)) == -1) 
		{
			perror("accept");
			exit(-1);
		}
		memset(&buf, 0, sizeof(buf));
		if (read(connet_fd, &buf, sizeof(buf)) == -1) 
		{
			perror("read");
			exit(-1);
		}

		for (i = 0; i < strlen(buf); i++) 
		{
			buf[i] = toupper(buf[i]);
		}
		if (write(connet_fd, &buf, sizeof(buf)) == -1) 
		{
			perror("write");
			exit(-1);
		}

		close(connet_fd);
	}

	close(socket_fd);
	return 0;
}
