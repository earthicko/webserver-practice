#ifndef POLLCPLISTENER_HPP
#define POLLCPLISTENER_HPP

#include "TCPListener.hpp"
#include <poll.h>

class PollTCPListener : public TCPListener
{
private:
	std::deque<struct pollfd> m_pollfds;

public:
	PollTCPListener(int port = 80);
	~PollTCPListener();
	PollTCPListener(const PollTCPListener &orig);
	PollTCPListener &operator=(const PollTCPListener &orig);

	virtual void initializeEventQueue(void);
	virtual void flushEventQueue(void);
	virtual void removeFromEventQueue(int fd);
};

#endif
