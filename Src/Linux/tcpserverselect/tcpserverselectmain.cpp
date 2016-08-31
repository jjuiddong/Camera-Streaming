//
// 2016-02-06, jjuiddong
//
// linux tcp server using select() function
//

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

using namespace std;

const int TCP_PORT = 5100;

int main()
{
	int ssock;
	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(TCP_PORT);

	if (bind(ssock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("bind()");
		return -1;
	}

	if (listen(ssock, 8) < 0)
	{
		perror("listen()");
		return -1;
	}

	cout << "tcp server select bind success... port:" << TCP_PORT << ", now listen.." << endl;

	struct sockaddr_in client_addr;
	int client_index = 0;
	int client_fd[5];

	while (1)
	{
		fd_set readfd;
		FD_ZERO(&readfd);
		FD_SET(ssock, &readfd);
		int maxfd = ssock;

		for (int i= 0; i < client_index; i++)
		{
			FD_SET(client_fd[i], &readfd);
			if (client_fd[i] > maxfd)
				maxfd = client_fd[i];
		}
		maxfd = maxfd + 1;

		select(maxfd, &readfd, NULL, NULL, NULL);
		if (FD_ISSET(ssock, &readfd))
		{
			socklen_t clen = sizeof(struct sockaddr_in);
			const int csock = accept(ssock, (struct sockaddr*)&client_addr, &clen);
			if (csock < 0)
			{
				perror("accept error\n");
				return -1;
			}
			else
			{
				FD_SET(csock, &readfd);
				client_fd[client_index] = csock;
				client_index++;
				cout << "Client is connected : " << inet_ntoa(client_addr.sin_addr) << ", fd : " << csock << endl;
				continue;
			}

			if (client_index >= 5)
				break;
		}

		for (int i = 0; i < client_index; ++i)
		{
			if (FD_ISSET(client_fd[i], &readfd))
			{
				char buf[BUFSIZ] = "Hello world";
				bzero(buf, sizeof(buf));
				int n = 0;
				if ((n = read(client_fd[i], buf, sizeof(buf))) > 0)
				{
					cout << "Received from FD : " << client_fd[i] << endl;
					cout << "Received Data Size  : " << n << endl;
					cout << "Received data : " << buf << endl;

					if (write(client_fd[i], buf, n) <= 0)
						perror("write()");

					close(client_fd[i]);
					FD_CLR(client_fd[i], &readfd);
					client_index--;
				}
			}

		}
	}

	close(ssock);

	return 0;
}
