#ifndef SELECTTCPLISTENER_HPP
#define SELECTTCPLISTENER_HPP

#include "TCPListener.hpp"
#include <sys/select.h>

class SelectTCPListener : public TCPListener
{
private:
	static const timeval zeroSecond;

public:
	SelectTCPListener(int port = 80);
	~SelectTCPListener();
	SelectTCPListener(const SelectTCPListener &orig);
	SelectTCPListener &operator=(const SelectTCPListener &orig);

	virtual void initializeEventQueue(void);
	virtual void flushEventQueue(void);
	virtual void removeFromEventQueue(int fd);
};

#endif
