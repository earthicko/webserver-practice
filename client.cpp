#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include "settings.h"

int main(int argc, char **argv)
{
	if (argc <= 1)
	{
		std::cerr << "need args" << std::endl;
		return (1);
	}
	std::cout << "port " << PORT << " buf size " << BUF_SIZE << std::endl;

	struct addrinfo hints, *res0;
	int error;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo("localhost", "http", &hints, &res0);
	if (error)
	{
		std::cerr << gai_strerror(error) << std::endl;
		return (1);
	}
	struct addrinfo *res = res0;
	while (res)
	{
		std::cout << "flags " << res->ai_flags
				  << " family " << res->ai_family
				  << " socktype " << res->ai_socktype
				  << " protocol " << res->ai_protocol
				  //   << " canonname " << std::string(res->ai_canonname, res->ai_canonname + 1)
				  << " addr " << res->ai_addr << std::endl;
		if (res->ai_family == PF_INET)
			break;
		res = res->ai_next;
	}
	if (!res)
	{
		std::cerr << "Could not find matching server" << std::endl;
		freeaddrinfo(res0);
		return (1);
	}
	int socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socketfd < 0)
	{
		std::cout << "Could not create socket" << std::endl;
		return (1);
	}
	int connectstat = connect(socketfd, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res0);
	if (connectstat < 0)
	{
		std::cerr << "Could not connect to server" << std::endl;
		perror(NULL);
		close(socketfd);
		return (1);
	}
	argv++;
	while (*argv)
	{
		ssize_t sendbytes = send(socketfd, *argv, std::strlen(*argv), 0);
		if (sendbytes < 0)
		{
			std::cerr << "Could not send to server" << std::endl;
			perror(NULL);
		}
		else
		{
			std::cout << "Sent " << sendbytes << " bytes" << std::endl;
		}
		char buf[BUF_SIZE + 1];
		ssize_t recvbytes = recv(socketfd, buf, BUF_SIZE, 0);
		if (recvbytes < 0)
		{
			std::cerr << "Could not receive from server" << std::endl;
			perror(NULL);
		}
		else
		{
			buf[recvbytes] = '\0';
			std::cout << "Received " << recvbytes << " bytes: " << buf << std::endl;
		}
		argv++;
	}
	close(socketfd);
	return (0);
}
