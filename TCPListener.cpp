#include "TCPListener.hpp"
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

const int TCPListener::m_backlog = 8;
const size_t TCPListener::m_buffsize = 2048;
const timespec zeroSecond = {0, 0};

void TCPListener::initialize(void)
{
	int result;
	m_socketListening = socket(PF_INET, SOCK_STREAM, 0);
	if (m_socketListening < 0)
		throw(std::runtime_error(strerror(errno)));
	std::cout << "Created socket " << m_socketListening << "\n";
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);
	result = bind(m_socketListening, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (result < 0)
		finalize(strerror(errno));
	std::cout << "Bind socket " << m_socketListening << " at port " << m_port << "\n";
	result = listen(m_socketListening, m_backlog);
	if (result < 0)
		finalize(strerror(errno));
	std::cout << "Listen with backlog size " << m_backlog << "\n";
	result = fcntl(m_socketListening, F_SETFL, O_NONBLOCK);
	if (result < 0)
		finalize(strerror(errno));
	m_kq = kqueue();
	if (m_kq < 0)
		finalize(strerror(errno));
	appendChangelist(m_socketListening, EVFILT_READ);
	flushKqueue();
	std::cout << "TCPListener initialization complete\n";
}

void TCPListener::finalize(const char *error)
{
	close(m_socketListening);
	m_socketListening = 0;
	if (error)
		throw(std::runtime_error(error));
}

/**
 * @brief Adds event filter to kqueue.
 *
 * @param ident descripter to watch.
 * @param filter Event to watch, EVFILT_READ or EVFILT_WRITE
 * @return Nothing.
 */
void TCPListener::appendChangelist(uintptr_t ident, int16_t filter)
{
	kevent_t temp;
	EV_SET(&temp, ident, filter, EV_ADD | EV_ENABLE, 0, 0, NULL);
	m_changelist.push_back(temp);
}

void TCPListener::flushKqueue(void)
{
	static const int eventbufSize = 8;
	kevent_t events[eventbufSize];
	int neweventCount = kevent(m_kq, &m_changelist[0], m_changelist.size(), events, eventbufSize, &zeroSecond);
	if (neweventCount < 0)
		throw(std::runtime_error(strerror(errno)));
	m_changelist.clear();
	for (int i = 0; i < neweventCount; i++)
		m_eventlist.push_back(events[i]);
}

void TCPListener::disconnect(int fd)
{
	close(fd);
	rdbuf.erase(fd);
	wrbuf.erase(fd);
	std::cout << "Disconnected " << fd << "\n";
}

void TCPListener::acceptNewClient(void)
{
	int newClientFd = accept(m_socketListening, NULL, NULL);
	if (newClientFd < 0)
		finalize(strerror(errno));
	std::cout << "Accepted new client: " << newClientFd << "\n";
	int result = fcntl(newClientFd, F_SETFL, O_NONBLOCK);
	if (result < 0)
		finalize(strerror(errno));
	appendChangelist(newClientFd, EVFILT_READ);
	appendChangelist(newClientFd, EVFILT_WRITE);
	rdbuf[newClientFd] = "";
	wrbuf[newClientFd] = "";
}

void TCPListener::recvFromClient(const int fd)
{
	char buff[m_buffsize + 1];
	ssize_t recvsize = recv(fd, buff, m_buffsize, 0);
	if (recvsize <= 0)
	{
		if (recvsize < 0)
			std::cerr << "Error while receiving from client " << strerror(errno) << "\n";
		disconnect(fd);
		return;
	}
	buff[recvsize] = '\0';
	rdbuf[fd] += buff;
	std::cerr << "Received \"" << buff << "\"\n";
}

void TCPListener::sendToClient(const int fd)
{
	ssize_t sendsize = send(fd, wrbuf[fd].c_str(), wrbuf[fd].length(), 0);
	if (sendsize < 0)
	{
		if (sendsize < 0)
			std::cerr << "Error while receiving from client " << strerror(errno) << "\n";
		disconnect(fd);
		return;
	}
	std::cout << "Send " << sendsize << " bytes: " << wrbuf[fd].substr(0, sendsize) << "\n";
	wrbuf[fd] = wrbuf[fd].substr(sendsize, wrbuf[fd].length());
}

TCPListener::TCPListener(int port)
	: m_port(port)
{
	initialize();
}

TCPListener::~TCPListener()
{
	finalize(NULL);
}

TCPListener::TCPListener(const TCPListener &orig)
	: m_port(orig.m_port)
{
	initialize();
}

TCPListener &TCPListener::operator=(const TCPListener &orig)
{
	finalize(NULL);
	m_port = orig.m_port;
	initialize();
	return (*this);
}

void TCPListener::task(void)
{
	flushKqueue();
	while (!m_eventlist.empty())
	{
		kevent_t event = m_eventlist.front();
		m_eventlist.pop_front();
		if (event.flags & EV_ERROR)
		{
			if (static_cast<int>(event.ident) == m_socketListening)
				finalize("Error from server socket");
			else
			{
				std::cerr << "Error from client socket" << std::endl;
				disconnect(event.ident);
			}
		}
		else if (event.filter == EVFILT_READ)
		{
			if (static_cast<int>(event.ident) == m_socketListening)
				acceptNewClient();
			else
				recvFromClient(event.ident);
		}
		else if (event.filter == EVFILT_WRITE)
		{
			if (wrbuf[event.ident].length() > 0)
				sendToClient(event.ident);
		}
	}
}
