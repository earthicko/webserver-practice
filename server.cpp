#include <iostream>
#include "TCPListener.hpp"

int main(void)
{
	TCPListener tcplistener;
	while (true)
	{
		tcplistener.task();
		for (TCPListener::iterator it = tcplistener.rdbuf.begin(); it != tcplistener.rdbuf.end(); it++)
		{
			int fd = it->first;
			if (tcplistener.rdbuf[fd].length() == 0)
				continue;
			std::cout << "Write to buf of " << fd << ":\"" << tcplistener.rdbuf[fd] << "\"\n";
			tcplistener.wrbuf[fd] = tcplistener.wrbuf[fd] + tcplistener.rdbuf[fd];
			tcplistener.rdbuf[fd] = "";
		}
	}
	return (0);
}
