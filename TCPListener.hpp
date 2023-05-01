#ifndef TCPLISTENER_HPP
#define TCPLISTENER_HPP

#include <map>
#include <deque>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

typedef struct kevent kevent_t;

class TCPListener
{
private:
	static const int m_backlog;
	static const size_t m_buffsize;
	int m_port;
	int m_socketListening;
	int m_kq;
	std::deque<kevent_t> m_changelist;
	std::deque<kevent_t> m_eventlist;

	void initialize(void);
	void finalize(const char *error);
	void appendChangelist(uintptr_t ident, int16_t filter);
	void flushKqueue(void);
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
	std::vector<int> getClients(void);
};

#endif
