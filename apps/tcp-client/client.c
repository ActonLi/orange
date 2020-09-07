#include "client.h"
#include <stdio.h>

typedef struct sockaddr common_addr;
typedef struct sockaddr_in myinet_addr;
int main(int argc, char *argv[])
{
	int client_fd;
	myinet_addr server_addr;
	char buf[MAXBUF] = "";
	if ((client_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(-1);
	}
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8000);
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.17");

	if (connect(client_fd, (common_addr*)(&server_addr), sizeof(server_addr)) == -1) 
	{
		perror("connect");
		exit(-1);
	}

    while(1) {
        sleep(1);
    }

	memset(&buf, 0, sizeof(buf));
	if (fgets(buf, MAXBUF, stdin) == NULL) 
	{
		perror("fgets");
		exit(-1);
	}
	if (write(client_fd, buf, sizeof(buf)) == -1) 
	{
		perror("write");
		exit(-1);
	}
	memset(&buf, 0, sizeof(buf));
	if (read(client_fd, buf, sizeof(buf)) == -1) 
	{
		perror("read");
		exit(-1);
	}
	printf("the string from server is: %s\n", buf);

	close(client_fd);
	return 0;
}
