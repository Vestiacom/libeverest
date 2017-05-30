#ifndef EVEREST_SERVER_HPP_
#define EVEREST_SERVER_HPP_


#include <ev++.h>
#include <functional>
#include <unordered_map>

#include "internals/acceptor.hpp"
#include "internals/connection.hpp"

namespace everest {

/**
 * This is the entry point of the library.
 * Pass callbacks for particular endpoints. 
 */
struct Server {
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

private:
	// A list of all active connections
	std::unordered_map<int, internals::Connection> mConnectionsIO;

	// Event loop
	struct ev_loop* mEvLoop;

	// Handles incoming connections
	internals::Acceptor mAcceptor;

	// Called when new connection is established.
	void onNewConnection(int fd);

	// When connection lost
	void onConectionLost(int fd);

};

} // namespace everest

#endif // EVEREST_SERVER_HPP_