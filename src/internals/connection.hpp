#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include "receiver.hpp"
#include "sender.hpp"
#include "../request.hpp"

#include <queue>
#include <vector>
#include <functional>
#include <memory>
#include <ev++.h>

namespace everest {

class Request;

namespace internals {

class Receiver;
class Sender;

/**
 * Wraps together all data used by one connection and HTTP parsing for this connection.
 *
 * Connections should be referenced only by std::shared_ptr,
 * because the Requests stores reference to the connection in a std::shared_ptr.
 * This way library user can always respond to a request. Only by having the Request object.
 *
 * TODO: graceful shutdown. Allow sending out data with timeout
 *
 */
struct Connection: std::enable_shared_from_this<Connection> {
	typedef std::function<void(const std::shared_ptr<Request>& request)> InputDataCallback;

	Connection(int fd,
	           struct ev_loop* evLoop,
	           const InputDataCallback& inputDataCallback);
	~Connection();

	Connection(const Connection&) = delete;
	Connection(Connection&&) = delete;
	Connection& operator=(const Connection&) = delete;

	/**
	 * Starts receiving and sending data
	 */
	void start();

	/**
	 * Stops receiving and sending data
	 */
	void stop();

	/**
	 * Write data to the output buffer.
	 * Will not block.
	 */
	void send(const std::shared_ptr<Response>& bufferPtr);

	/**
	 * @return underlying socket's fd
	 */
	int getFD();

	/**
	 * @return is this connection closed
	 */
	bool isClosed();

private:
	// Handles receiving data and parsing HTTP requests
	Receiver mReceiver;

	// Handles sending HTTP responses
	Sender mSender;

	// Shutdowns the connection (calls ConnectionLostCallback)
	void shutdown();

	// Socket's fd
	int mFD;

};

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_CONNECTION_HPP_