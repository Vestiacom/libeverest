#ifndef EVEREST_SERVER_HPP_
#define EVEREST_SERVER_HPP_


#include <ev++.h>
#include <functional>
#include <unordered_map>

#include "internals/acceptor.hpp"
#include "internals/connection.hpp"
#include "internals/common.hpp"

#include "request.hpp"

namespace everest {

class Request;

/**
 * This is the entry point of the library.
 * Pass callbacks for particular endpoints.
 */
struct EVEREST_API Server {
	typedef std::function<void(const std::shared_ptr<Request>&)> EndpointCallback;

	Server(const unsigned short port, struct ev_loop* evLoop);
	~Server();

	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;

	/**
	 * Starts serving requests using the ev loop
	 *
	 * @param ev_loop ev loop
	 */
	void start();

	/**
	 * Stops serving requests.
	 */
	void stop();

	/**
	 * Registers callback for given URL
	 *
	 * @param url endpoint's URL
	 * @param endpointCallback called when there's a request for this URL
	 */
	void endpoint(const std::string& url, const EndpointCallback& endpointCallback);

	/**
	 * @return number of opened connections
	 */
	std::size_t getConnectionsNumber();

private:
	// A list of all active connections
	std::unordered_map<int, std::shared_ptr<internals::Connection>> mConnections;

	// Endpoints connected to the
	std::unordered_map<std::string, EndpointCallback> mEndpointCallbacks;

	// Event loop
	struct ev_loop* mEvLoop;

	// Handles incoming connections
	internals::Acceptor mAcceptor;

	// Called when new connection is established.
	EVEREST_LOCAL void onNewConnection(int fd);

	// When connection is lost
	EVEREST_LOCAL void onConnectionLost(int fd);

	// New request arrived
	EVEREST_LOCAL void onNewRequest(const std::shared_ptr<Request>& r);
};

} // namespace everest

#endif // EVEREST_SERVER_HPP_