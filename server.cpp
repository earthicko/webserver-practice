#include <iostream>
#include "TCPListener.hpp"

int main(void)
{
	KQueueTCPListener listener;
	while (true)
	{
		listener.task();
		for (KQueueTCPListener::iterator it = listener.rdbuf.begin(); it != listener.rdbuf.end(); it++)
		{
			int fd = it->first;
			if (listener.rdbuf[fd].length() == 0)
				continue;
			std::cout << "Write to buf of " << fd << ":\"" << listener.rdbuf[fd] << "\"\n";
			listener.wrbuf[fd] = listener.wrbuf[fd] + listener.rdbuf[fd];
			listener.rdbuf[fd] = "";
		}
	}
	return (0);
}
