#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include "receiver.hpp"
#include "../request.hpp"

#include <queue>
#include <vector>
#include <functional>
#include <memory>
#include <ev++.h>
#include <http_parser.h>

namespace everest {

class Request;
class Response;

namespace internals {

class Receiver;

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

	// Pop the oldest Response and serialize it to the output buffer
	void fillBuffer();

	// Shutdowns the connection (calls ConnectionLostCallback)
	void shutdown();

	// Socket ready to receive data
	void onOutput(ev::io& w, int revents);

	// Waits when OS is ready to receive data do send
	ev::io mOutputWatcher;

	// Socket's fd
	int mFD;

	// When Response is send it serializes into a buffer and gets stored here.
	// Responses are send in order, one by one, in the onOutput callback.
	std::queue<std::shared_ptr<Response>> mResponses;

	// Buffer with the latest response to send
	std::vector<char> mOutputBuffer;

	// Position in the latest buffer.
	// Some data might have been send - this is the position of the unsent data.
	std::size_t mOutputBufferPosition;
};

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_CONNECTION_HPP_