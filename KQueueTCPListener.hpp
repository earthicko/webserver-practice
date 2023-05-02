#ifndef KQUEUETCPLISTENER_HPP
#define KQUEUETCPLISTENER_HPP

#include "TCPListener.hpp"

class KQueueTCPListener : public TCPListener
{
private:
	int m_kq;
	static const timespec zeroSecond;

public:
	KQueueTCPListener(int port = 80);
	~KQueueTCPListener();
	KQueueTCPListener(const KQueueTCPListener &orig);
	KQueueTCPListener &operator=(const KQueueTCPListener &orig);

	virtual void initializeEventQueue(void);
	virtual void flushEventQueue(void);
	virtual void removeFromEventQueue(int fd);
};

#endif
