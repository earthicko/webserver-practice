#include "SelectTCPListener.hpp"
#include <errno.h>

const timeval SelectTCPListener::zeroSecond = {0, 0};

void SelectTCPListener::initializeEventQueue(void)
{
}

void SelectTCPListener::flushEventQueue(void)
{
	struct fd_set rdfds;
	struct fd_set wrfds;
	struct fd_set erfds;
	FD_ZERO(&rdfds);
	FD_ZERO(&wrfds);
	FD_ZERO(&erfds);
	static int nfds;
	for (std::deque<TCPIOEvent>::iterator it = m_watchlist.begin(); it != m_watchlist.end(); it++)
	{
		switch (it->event)
		{
		case READ:
			FD_SET(it->fd, &rdfds);
			break;
		case WRITE:
			FD_SET(it->fd, &wrfds);
			break;
		default:
			throw(std::logic_error("watchlist shouldn't contain ERROR events."));
			break;
		}
		FD_SET(it->fd, &erfds);
		nfds = std::max(nfds, it->fd + 1);
	}
	int result = select(nfds, &rdfds, &wrfds, &erfds, (struct timeval *)(&zeroSecond));
	if (result < 0)
		throw(std::runtime_error(strerror(errno)));
	if (result == 0)
		return;
	for (int fd = 0; fd < nfds; fd++)
	{
		if (FD_ISSET(fd, &rdfds))
			m_eventlist.push_back(TCPIOEvent(fd, READ));
		if (FD_ISSET(fd, &wrfds))
			m_eventlist.push_back(TCPIOEvent(fd, WRITE));
		if (FD_ISSET(fd, &erfds))
			m_eventlist.push_back(TCPIOEvent(fd, ERROR));
	}
}

void SelectTCPListener::removeFromEventQueue(int fd)
{
	std::deque<TCPIOEvent>::size_type initialSize = m_watchlist.size();
	for (std::deque<TCPIOEvent>::size_type i = 0; i < initialSize; i++)
	{
		TCPIOEvent temp = m_watchlist.front();
		m_watchlist.pop_front();
		if (temp.fd != fd)
			m_watchlist.push_back(temp);
	}
}

SelectTCPListener::SelectTCPListener(int port)
	: TCPListener(port)
{
	initialize();
}

SelectTCPListener::~SelectTCPListener()
{
	finalize(NULL);
}

SelectTCPListener::SelectTCPListener(const SelectTCPListener &orig)
	: TCPListener(orig)
{
	initialize();
}

SelectTCPListener &SelectTCPListener::operator=(const SelectTCPListener &orig)
{
	finalize(NULL);
	TCPListener::operator=(orig);
	initialize();
	return (*this);
}
