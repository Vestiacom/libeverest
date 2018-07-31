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

Server::Server(const Config& config, struct ev_loop* evLoop)
	: mEvLoop(evLoop),
	  mConfig(config),
	  mCleanupTimer(evLoop),
	  mAcceptor(config.ip, config.port, evLoop, std::bind(&Server::onNewConnection, this, _1))
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
	mCleanupTimer.start(0, mConfig.cleanupPeriodSec);
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
	if (mConnections.size() > mConfig.maxConnections) {
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
		try {
			LOGD("New request on connection fd: " << r->getFD());

			auto& callback = mEndpointCallbacks.at(r->getURL());

			try {
				callback(r);
			}
			catch (const std::exception& e) {
				LOGE("Exception in endpoint: " << r->getURL() << ", fd: " << r->getFD() << " :" << e.what());
				auto resp = r->createResponse();
				resp->setStatus(500);
				resp->setClosing(true);
				resp->send();
			}
		}
		catch (const std::out_of_range&) {
			LOGE("No callback for URL: " << r->getURL() << ", returning 404, fd: "  << r->getFD());
			auto resp = r->createResponse();
			resp->setStatus(404);
			resp->setClosing(true);
			resp->send();
		}
	}
	catch (const std::exception& e) {
		// Most probably send from the catch threw
		LOGE("Unexpected exception in endpoint handler:" << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
}

void Server::replyToExpired()
{
	using duration = std::chrono::duration<double>;

	auto now = std::chrono::steady_clock::now();
	for (auto& c : mConnections) {
		if (c->isClosed()) {
			continue;
		}

		const duration sinceStart = now - c->getStartTime();
		if (sinceStart.count() < mConfig.maxRequestTimeSec) {
			// Not old enough to send timeoout
			continue;
		}

		LOGW("Connection timed out, responding 408 Request Timeout. fd: " << c->getFD());

		try {
			auto resp = std::make_shared<Response>(c, true /* close after sending this reply */);
			resp->setClosing(true);
			resp->setStatus(408); // 408 Request Timeout
			resp->send();
		}
		catch (const std::exception& e) {
			LOGW("Exception when sending a reply to expired request: " << e.what());
		}
	}
}

void Server::removeDisconnected()
{
	mConnections.erase(std::remove_if(mConnections.begin(),
	                                  mConnections.end(),
	[this](const std::shared_ptr<internals::Connection> c) {
		if (!c || c->isClosed()) {
			LOGD("Garbage collecting connection, fd: " << c->getFD());
			return true;
		}

		return false;
	}), mConnections.end());

	// Stop or resume accepting more connections
	mConnections.size() > mConfig.maxConnections ? mAcceptor.stop() : mAcceptor.start();
}

void Server::onCleanupTimeout(ev::timer&, int)
{
	try {
		removeDisconnected();
		replyToExpired();
		mConnections.shrink_to_fit();
	}
	catch (const std::exception& e) {
		LOGE("Unexpected exception: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
}

std::size_t Server::getConnectionsNumber() const
{
	return mConnections.size();
}


} // namespace everest
