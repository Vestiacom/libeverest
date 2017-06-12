#include "server.hpp"
#include "internals/logger.hpp"

#include <stdexcept>
#include <string>
#include <algorithm>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include "internals/connection.hpp"

using namespace std::placeholders;

namespace everest {

Server::Server(const unsigned short port, struct ev_loop* evLoop, const size_t maxConnections)
	: mEvLoop(evLoop),
	  mMaxConnections(maxConnections),
	  mCleanupTimer(evLoop),
	  mAcceptor(port, evLoop, std::bind(&Server::onNewConnection, this, _1))
{
	mCleanupTimer.set<Server, &Server::onCleanupTimeout>(this);
}

Server::~Server()
{
	stop();
}

void Server::start()
{
	mAcceptor.start();
	mCleanupTimer.start(0.25/*after in seconds*/, 0.25 /*period in seconds*/);
}

void Server::stop()
{
	mCleanupTimer.stop();
	mAcceptor.stop();
	mConnections.clear();
}

void Server::endpoint(const std::string& url, const EndpointCallback& endpointCallback)
{
	mEndpointCallbacks[url] = endpointCallback;
}

void Server::onNewConnection(int fd)
{
	if (mConnections.size() > mMaxConnections) {
		// Stop accepting more connections
		mAcceptor.stop();
		removeDisconnected();
	}

	// Acceptor accepted a new connection
	auto connection = std::make_shared<internals::Connection>(fd,
	                                                          mEvLoop,
	                                                          std::bind(&Server::onNewRequest, this, _1));
	connection->start();
	mConnections.push_back(connection);
}

void Server::onNewRequest(const std::shared_ptr<Request>& r)
{
	try {
		mEndpointCallbacks[r->getURL()](r);
	} catch (const std::out_of_range&) {
		// No callback for this URL
		LOGE("No callback for URL, returning 404");
		// auto resp = r.createResponse();
		// resp->setStatus(404);
		// resp->send();
	}
}

void Server::removeDisconnected()
{
	mConnections.erase(std::remove_if(mConnections.begin(),
	                                  mConnections.end(),
	[this](const std::shared_ptr<internals::Connection> c) {
		if (!c) {
			return true;
		}
		return c->isClosed();
	}), mConnections.end());

	// Stop or resume accepting more connections
	mConnections.size() > mMaxConnections ? mAcceptor.stop() : mAcceptor.start();
}

void Server::onCleanupTimeout(ev::timer&, int)
{
	removeDisconnected();
	mConnections.shrink_to_fit();
}

std::size_t Server::getConnectionsNumber() const
{
	return mConnections.size();
}


} // namespace everest
