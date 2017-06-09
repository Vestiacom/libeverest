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

// TODO: Remove
#include <iostream>
using namespace std;


namespace everest {

Server::Server(const unsigned short port, struct ev_loop* evLoop)
	: mEvLoop(evLoop),
	  mAcceptor(port, evLoop, std::bind(&Server::onNewConnection, this, _1))
{

}

Server::~Server()
{
	stop();

}

void Server::start()
{
	mAcceptor.start();
}

void Server::stop()
{
	mAcceptor.stop();
	for (auto& connection : mConnections) {
		connection.second->stop();
	}
}

void Server::endpoint(const std::string& url, const EndpointCallback& endpointCallback)
{
	mEndpointCallbacks[url] = endpointCallback;
}

void Server::onNewConnection(int fd)
{
	// Acceptor accepted a new connection
	auto connection = std::make_shared<internals::Connection>(fd,
	                                                          mEvLoop,
	                                                          std::bind(&Server::onNewRequest, this, _1),
	                                                          std::bind(&Server::onConnectionLost, this, _1));
	connection->start();
	mConnections[fd] = connection;
}

void Server::onNewRequest(const std::shared_ptr<Request>& r)
{
	try {
		mEndpointCallbacks[r->getURL()](r);
	} catch (const std::out_of_range&) {
		// No callback for this URL
	}
}

void Server::onConnectionLost(int fd)
{
	mConnections.erase(fd);
}

std::size_t Server::getConnectionsNumber()
{
	return mConnections.size();
}


} // namespace everest
