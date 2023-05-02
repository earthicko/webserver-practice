#ifndef KQUEUETCPLISTENER_HPP
#define KQUEUETCPLISTENER_HPP

#include "TCPListener.hpp"

class KQueueTCPListener : public TCPListener
{
public:
	int m_kq;
	static const timespec zeroSecond;

	KQueueTCPListener(int port = 80);
	~KQueueTCPListener();
	KQueueTCPListener(const KQueueTCPListener &orig);
	KQueueTCPListener &operator=(const KQueueTCPListener &orig);

	virtual void initializeEventQueue(void);
	virtual void flushEventQueue(void);
};

#endif
