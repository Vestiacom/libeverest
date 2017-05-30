#include "acceptor.hpp"

#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>

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
		std::ostringstream msg;
		msg << "setsockopt() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		std::ostringstream msg;
		msg << "bind() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::listen(fd, 10) == -1) {
		std::ostringstream msg;
		msg << "listen() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}
}

} // namespace

namespace everest {
namespace internals {

Acceptor::Acceptor(const unsigned short port, struct ev_loop* evLoop)
	: mWatcher(evLoop)
{
	mWatcher.set<Acceptor, &Acceptor::onNewConnection>(this);
	mFD = createListeningSocket();
}
Acceptor::~Acceptor()
{
}

void Acceptor::start()
{
	mWatcher.start(mFD, ev::READ);
}

void Acceptor::stop()
{
	mWatcher.stop();
}

Acceptor::onNewConnection(ev::io& w, int revents)
{

}

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_ACCEPTOR_HPP_