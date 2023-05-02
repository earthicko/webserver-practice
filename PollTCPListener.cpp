#include "PollTCPListener.hpp"
#include <errno.h>

void PollTCPListener::initializeEventQueue(void)
{
}

void PollTCPListener::flushEventQueue(void)
{
	for (std::deque<TCPIOEvent>::iterator ioit = m_watchlist.begin(); ioit != m_watchlist.end(); ioit++)
	{
		bool foundDup = false;
		for (std::deque<struct pollfd>::iterator pollit = m_pollfds.begin(); pollit != m_pollfds.end(); pollit++)
		{
			if (pollit->fd == ioit->fd)
			{
				switch (ioit->event)
				{
				case READ:
					pollit->events |= POLLIN;
					break;
				case WRITE:
					pollit->events |= POLLOUT;
					break;
				default:
					throw(std::logic_error("watchlist shouldn't contain ERROR events."));
					break;
				}
				foundDup = true;
				break;
			}
		}
		if (foundDup)
			continue;
		struct pollfd temp;
		temp.fd = ioit->fd;
		temp.events = 0;
		switch (ioit->event)
		{
		case READ:
			temp.events |= POLLIN;
			break;
		case WRITE:
			temp.events |= POLLOUT;
			break;
		default:
			throw(std::logic_error("watchlist shouldn't contain ERROR events."));
			break;
		}
		m_pollfds.push_back(temp);
	}
	m_watchlist.clear();
	if (m_pollfds.empty())
		return;
	int result = poll(&m_pollfds[0], m_pollfds.size(), 0);
	if (result < 0)
		throw(std::runtime_error(strerror(errno)));
	if (result == 0)
		return;
	for (std::deque<struct pollfd>::iterator pollit = m_pollfds.begin(); pollit != m_pollfds.end(); pollit++)
	{
		if (pollit->revents & (POLLERR | POLLNVAL))
			m_eventlist.push_back(TCPIOEvent(pollit->fd, ERROR));
		if (pollit->revents & POLLIN)
			m_eventlist.push_back(TCPIOEvent(pollit->fd, READ));
		if (pollit->revents & POLLOUT)
			m_eventlist.push_back(TCPIOEvent(pollit->fd, WRITE));
	}
}

void PollTCPListener::removeFromEventQueue(int fd)
{
	for (std::deque<struct pollfd>::iterator pollit = m_pollfds.begin(); pollit != m_pollfds.end(); pollit++)
	{
		if (pollit->fd == fd)
		{
			m_pollfds.erase(pollit);
			return;
		}
	}
}

PollTCPListener::PollTCPListener(int port)
	: TCPListener(port)
{
	initialize();
}

PollTCPListener::~PollTCPListener()
{
	finalize(NULL);
}

PollTCPListener::PollTCPListener(const PollTCPListener &orig)
	: TCPListener(orig)
{
	initialize();
}

PollTCPListener &PollTCPListener::operator=(const PollTCPListener &orig)
{
	finalize(NULL);
	TCPListener::operator=(orig);
	initialize();
	return (*this);
}
