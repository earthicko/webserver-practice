#include "KQueueTCPListener.hpp"
#include <errno.h>

const timespec KQueueTCPListener::zeroSecond = {0, 0};

void KQueueTCPListener::initializeEventQueue(void)
{
	m_kq = kqueue();
	if (m_kq < 0)
		finalize(strerror(errno));
}

void KQueueTCPListener::flushEventQueue(void)
{
	static const int eventbufSize = 8;
	struct kevent events[eventbufSize];
	int neweventCount;
	if (m_watchlist.empty())
		neweventCount = kevent(m_kq, NULL, 0, events, eventbufSize, &zeroSecond);
	else
	{
		std::deque<struct kevent> changelist;
		for (std::deque<TCPIOEvent>::iterator it = m_watchlist.begin(); it != m_watchlist.end(); it++)
			changelist.push_back(it->toKevent());
		neweventCount = kevent(m_kq, &changelist[0], changelist.size(), events, eventbufSize, &zeroSecond);
	}
	if (neweventCount < 0)
		throw(std::runtime_error(strerror(errno)));
	m_watchlist.clear();
	for (int i = 0; i < neweventCount; i++)
		m_eventlist.push_back(TCPIOEvent(events[i]));
}

void KQueueTCPListener::removeFromEventQueue(int fd)
{
	(void)fd;
}

KQueueTCPListener::KQueueTCPListener(int port)
	: TCPListener(port)
{
	initialize();
}

KQueueTCPListener::~KQueueTCPListener()
{
	finalize(NULL);
}

KQueueTCPListener::KQueueTCPListener(const KQueueTCPListener &orig)
	: TCPListener(orig)
{
	initialize();
}

KQueueTCPListener &KQueueTCPListener::operator=(const KQueueTCPListener &orig)
{
	finalize(NULL);
	TCPListener::operator=(orig);
	initialize();
	return (*this);
}
