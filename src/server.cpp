#include "server.hpp"

#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>



namespace everest {


Server::Server(const unsigned short port, struct ev_loop* evLoop)
	: mEvLoop(evLoop),
	  mIO(mEvLoop)
{
	mIO.set<Server, &Server::onEvent>(this);

	// Setup socket fd, but doesn't server requests yet
	createListeningSocket(port);
}

Server::~Server()
{

}

void Server::createListeningSocket(const unsigned short port)
{
	// setup socket for TCP proxy
	mFD = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mFD == -1) {
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
	if (::setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		std::ostringstream msg;
		msg << "setsockopt() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::bind(mFD, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		std::ostringstream msg;
		msg << "bind() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (::listen(mFD, 10) == -1) {
		std::ostringstream msg;
		msg << "listen() failed with: " << std::strerror(errno);
		throw std::runtime_error(msg.str());
	}
}

void Server::start()
{
	mIO.start(mFD, ev::READ);
}

void Server::stop()
{
	mIO.stop();
}

void Server::onEvent(ev::io& /*watcher*/ , int /*revents*/)
{
	// if (EV_ERROR & revents) {
	// 	return;
	// }

	// if (revents & EV_READ) {
	// 	onInput();
	// }

	// if (revents & EV_WRITE) {
	// 	onOutput();
	// }
}

// void Server::writeSafe(int fd, const void* bufferPtr, const size_t size)
// {
// 	size_t nTotal = 0;
// 	for (;;) {
// 		auto data = reinterpret_cast<const char*>(bufferPtr) + nTotal;

// 		int n  = ::write(fd, data, size - nTotal);
// 		if (n >= 0) {
// 			nTotal += n;
// 			if (nTotal == size) {
// 				// All data is written, break loop
// 				break;
// 			}
// 		} else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
// 			// Neglected errors
// 		} else {
// 			const std::string msg = "write() failed with: " + strerrorSafe();
// 			// LOG4CPLlUS_ERROR(logger, msg);

// 			// TODO: Throw
// 			return;
// 		}
// 	}
// }

// void  Server::onNewConnection()
// {

// }


// void Server::onInput(ev::io& watcher)
// {

// }

// void Server::onOutput(ev::io& watcher)
// {

// 	// if (writeBuffer.empty()) {
// 	// 	io.set(ev::READ);
// 	// } else {
// 	// 	io.set(ev::READ | ev::WRITE);
// 	// }
// }


} // namespace everest
