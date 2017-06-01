#include "acceptor.hpp"


#include <ev++.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>

namespace {

/**
 * @return Socket setup for listening for new HTTP connections
 */
int createListeningSocket(const unsigned short port)
{
	// setup socket for TCP proxy
	int fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		std::ostringstream msg;
		msg << "socket() failed with:  " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		::close(fd);

		std::ostringstream msg;
		msg << "setsockopt() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		::close(fd);

		std::ostringstream msg;
		msg << "bind() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::listen(fd, 10) == -1) {
		::close(fd);

		std::ostringstream msg;
		msg << "listen() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	return fd;
}

} // namespace

namespace everest {
namespace internals {

Acceptor::Acceptor(const unsigned short port,
                   struct ev_loop* evLoop,
                   const NewConnectionCallback& newConnectionCallback)
	: mWatcher(evLoop),
	  mNewConnectionCallback(newConnectionCallback)
{
	if (!evLoop) {
		throw std::runtime_error("ev_loop is null");
	}

	mFD = createListeningSocket(port);

	mWatcher.set<Acceptor, &Acceptor::onNewConnection>(this);
}

Acceptor::~Acceptor()
{
	mWatcher.stop();

	::close(mFD);
}

void Acceptor::start()
{
	mWatcher.start(mFD, ev::READ);
}

void Acceptor::stop()
{
	mWatcher.stop();
}

void Acceptor::onNewConnection(ev::io& w, int revents)
{
	// Unspecified error?
	if (EV_ERROR & revents) {
		return;
	}

	// New proxy connection
	int fd = ::accept4(w.fd, NULL, NULL, SOCK_NONBLOCK);
	if (fd < 0) {
		return;
	}

	if (mNewConnectionCallback) {
		mNewConnectionCallback(fd);
	}
}

} // namespace internals
} // namespace everest
