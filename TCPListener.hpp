#ifndef TCPLISTENER_HPP
#define TCPLISTENER_HPP

#include <map>
#include <deque>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

enum IOEVENT_E
{
	READ = 0,
	WRITE = 1,
	ERROR = 2
};

class TCPListener
{
private:
	class TCPIOEvent // implementation-independent I/O Event
	{
	public:
		int fd;
		int event;

		~TCPIOEvent();
		TCPIOEvent(const TCPIOEvent &orig);
		TCPIOEvent &operator=(const TCPIOEvent &orig);
		TCPIOEvent(int _fd, int _event);
		// implementation-specific conversions below
		TCPIOEvent(const struct kevent &orig);
		TCPIOEvent &operator=(const struct kevent &orig);

		struct kevent toKevent(void);

	private:
		TCPIOEvent(void);
	};

	static const int m_backlog;
	static const size_t m_buffsize;
	int m_port;
	int m_socketListening;
	int m_kq;
	std::deque<TCPIOEvent> m_watchlist;
	std::deque<TCPIOEvent> m_eventlist;

	void initialize(void);
	void initializeEventQueue(void);
	void finalize(const char *error);
	void flushEventQueue(void);
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
