#include "server.hpp"

#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include "internals/connection.hpp"

using namespace std::placeholders;

namespace everest {

Server::Server(const unsigned short port, struct ev_loop* evLoop)
	: mEvLoop(evLoop),
	  mAcceptor(port, evLoop, std::bind(&Server::onNewConnection, this, _1))
{

}

Server::~Server()
{
	mAcceptor.stop();
}

void Server::start()
{
	mAcceptor.start();
}

void Server::stop()
{
	mAcceptor.stop();
}


void Server::onNewConnection(int )
{

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
