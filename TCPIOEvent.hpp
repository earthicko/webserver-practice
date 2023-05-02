#ifndef TCPIOEVENT_HPP
#define TCPIOEVENT_HPP

#include <sys/types.h>
#include <sys/event.h>
#include <stdexcept>

enum IOEVENT_E
{
	READ = 0,
	WRITE = 1,
	ERROR = 2
};

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

#endif
