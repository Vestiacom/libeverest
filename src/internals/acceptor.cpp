#include "acceptor.hpp"
#include "logger.hpp"

#include <ev++.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace {

/**
 * @return Socket setup for listening for new HTTP connections
 */
int createListeningSocket(const std::string& ip, const unsigned short port)
{
	// setup socket for TCP proxy
	int fd = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		THROW("socket() failed with:  " << std::strerror(errno));
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	int optval = 1;
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		::close(fd);
		THROW("setsockopt() failed with: " << std::strerror(errno));
	}

	if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		::close(fd);
		THROW("bind() failed with: " << std::strerror(errno));
	}

	if (::listen(fd, 10) == -1) {
		::close(fd);
		THROW("listen() failed with: " << std::strerror(errno));
	}

	return fd;
}

} // namespace

namespace everest {
namespace internals {

Acceptor::Acceptor(const std::string& ip,
                   const unsigned short port,
                   struct ev_loop* evLoop,
                   const NewConnectionCallback& newConnectionCallback)
	: mWatcher(evLoop),
	  mNewConnectionCallback(newConnectionCallback)
{
	if (!evLoop) {
		THROW("ev_loop is null");
	}

	mFD = createListeningSocket(ip, port);

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
	try {
		LOGD("Accepting new connection");
		if (EV_ERROR & revents) {
			// Unspecified error
			LOGE("Unspecified on accepting socket");
		}

		// New proxy connection
		int fd = ::accept4(w.fd, NULL, NULL, SOCK_NONBLOCK);
		if (fd < 0) {
			LOGW("accept4() failed with " << std::strerror(errno));
			return;
		}

		if (mNewConnectionCallback) {
			mNewConnectionCallback(fd);
		}
	}
	catch (const std::exception& e) {
		LOGE("Unexpected exception: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
}

} // namespace internals
} // namespace everest
