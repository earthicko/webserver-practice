#ifndef TCPLISTENER_HPP
#define TCPLISTENER_HPP

#include <map>
#include <deque>
#include <sys/types.h>
#include <sys/event.h>

enum IOEVENT_E
{
	READ = 0,
	WRITE = 1,
	ERROR = 2
};

class TCPListener
{
private:
	static const int m_backlog;
	static const size_t m_buffsize;
	int m_port;
	int m_socketListening;

protected:
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
