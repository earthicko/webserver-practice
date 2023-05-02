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
	initializeEventQueue();
	m_watchlist.push_back(TCPListener::TCPIOEvent(m_socketListening, READ));
	flushEventQueue();
	std::cout << "TCPListener initialization complete\n";
}

void TCPListener::initializeEventQueue(void)
{
	m_kq = kqueue();
	if (m_kq < 0)
		finalize(strerror(errno));
}

void TCPListener::finalize(const char *error)
{
	close(m_socketListening);
	m_socketListening = 0;
	if (error)
		throw(std::runtime_error(error));
}

void TCPListener::flushEventQueue(void)
{
	static const int eventbufSize = 8;
	struct kevent events[eventbufSize];
	int neweventCount;
	if (m_watchlist.empty())
		neweventCount = kevent(m_kq, NULL, 0, events, eventbufSize, &zeroSecond);
	else
	{
		std::deque<struct kevent> changelist;
		for (std::deque<TCPIOEvent>::iterator it = m_watchlist.begin(); it != m_watchlist.end(); it++)
			changelist.push_back(it->toKevent());
		neweventCount = kevent(m_kq, &changelist[0], changelist.size(), events, eventbufSize, &zeroSecond);
	}
	if (neweventCount < 0)
		throw(std::runtime_error(strerror(errno)));
	m_watchlist.clear();
	for (int i = 0; i < neweventCount; i++)
		m_eventlist.push_back(TCPListener::TCPIOEvent(events[i]));
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
	m_watchlist.push_back(TCPListener::TCPIOEvent(newClientFd, READ));
	m_watchlist.push_back(TCPListener::TCPIOEvent(newClientFd, WRITE));
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
	flushEventQueue();
	while (!m_eventlist.empty())
	{
		TCPListener::TCPIOEvent event = m_eventlist.front();
		m_eventlist.pop_front();
		switch (event.event)
		{
		case ERROR:
			if (event.fd == m_socketListening)
				finalize("Error from server socket");
			else
			{
				std::cerr << "Error from client socket" << std::endl;
				disconnect(event.fd);
			}
			break;
		case READ:
			if (event.fd == m_socketListening)
				acceptNewClient();
			else
				recvFromClient(event.fd);
			break;
		case WRITE:
			if (wrbuf[event.fd].length() > 0)
				sendToClient(event.fd);
			break;
		default:
			break;
		}
	}
}
