#ifndef TCPLISTENER_HPP
#define TCPLISTENER_HPP

#include <map>
#include <deque>
#include "TCPIOEvent.hpp"

class TCPListener
{
private:
	static const int m_backlog;
	static const size_t m_buffsize;
	int m_port;
	int m_socketListening;

protected:
	std::deque<TCPIOEvent> m_watchlist;
	std::deque<TCPIOEvent> m_eventlist;

	void initialize(void);
	void finalize(const char *error);
	virtual void initializeEventQueue(void) = 0;
	virtual void flushEventQueue(void) = 0;
	void disconnect(int fd);
	void acceptNewClient(void);
	void recvFromClient(const int fd);
	void sendToClient(const int fd);

public:
	typedef std::map<int, std::string>::iterator iterator;
	std::map<int, std::string> rdbuf;
	std::map<int, std::string> wrbuf;

	TCPListener(int port = 80);
	~TCPListener();
	TCPListener(const TCPListener &orig);
	TCPListener &operator=(const TCPListener &orig);

	void task(void);
};

#endif
