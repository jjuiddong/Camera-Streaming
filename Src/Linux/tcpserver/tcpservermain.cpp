//
// 2016-02-06, jjuiddong
//
// linux tcp server 
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
	struct sockaddr_in server_addr;
	char buf[BUFSIZ] = "Hello world";

	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		return -1;
	}

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

	cout << "tcp server bind success... port:" << TCP_PORT << ", now listen.." << endl;

	struct sockaddr_in client_addr;
	socklen_t clen = sizeof(client_addr);
	while (1) 
	{
		int csock = accept(ssock, (struct sockaddr*)&client_addr, &clen);
		cout << "Client is connected : " << inet_ntoa(client_addr.sin_addr) << endl;

		int n;
		if ((n = read(csock, buf, BUFSIZ)) <= 0)
			perror("read()");

		cout << "Received Data Size  : " << n << endl;
		cout << "Received data : " << buf << endl;

		if (write(csock, buf, n) <= 0)
			perror("write()");

		close(csock);
	}

	close(ssock);

	return 0;
}
