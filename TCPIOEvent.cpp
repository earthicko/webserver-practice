#include "TCPListener.hpp"

TCPListener::TCPIOEvent::~TCPIOEvent()
{
}

TCPListener::TCPIOEvent::TCPIOEvent(const TCPIOEvent &orig)
	: fd(orig.fd), event(orig.event)
{
}

TCPListener::TCPIOEvent &TCPListener::TCPIOEvent::operator=(const TCPIOEvent &orig)
{
	fd = orig.fd;
	event = orig.event;
	return (*this);
}

TCPListener::TCPIOEvent::TCPIOEvent(int _fd, int _event)
	: fd(_fd), event(_event)
{
}

TCPListener::TCPIOEvent::TCPIOEvent(const struct kevent &orig)
{
	*this = orig;
}

TCPListener::TCPIOEvent &TCPListener::TCPIOEvent::operator=(const struct kevent &orig)
{
	fd = orig.ident;
	if (orig.flags & EV_ERROR)
		event = ERROR;
	else
	{
		switch (orig.filter)
		{
		case EVFILT_READ:
			event = READ;
			break;
		case EVFILT_WRITE:
			event = WRITE;
			break;
		default:
			throw(std::logic_error("Not implemented kevent filter."));
		}
	}
	return (*this);
}

struct kevent TCPListener::TCPIOEvent::toKevent(void)
{
	if (event == ERROR)
		throw(std::logic_error("TCPIOEvent with ERROR state cannot be converted into kevent."));
	struct kevent output;
	int filter;
	switch (event)
	{
	case READ:
		filter = EVFILT_READ;
		break;
	case WRITE:
		filter = EVFILT_WRITE;
		break;
	default:
		throw(std::logic_error("Not implemented TCPIOEvent."));
	}
	EV_SET(&output, fd, filter, EV_ADD | EV_ENABLE, 0, 0, NULL);
	return (output);
}

TCPListener::TCPIOEvent::TCPIOEvent(void)
	: fd(0), event(0)
{
}
