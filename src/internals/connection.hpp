#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

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

	// Http proxy initialization
	void setupHttpProxy();

	// Pop the oldest Request and serialize it to the output buffer
	void fillBuffer();

	// Headers parsing finished
	static int onHeadersComplete(::http_parser*);

	// New message parsing started
	static int onMessageBegin(::http_parser*);

	// Message is parsed and request is ready
	static int onMessageComplete(::http_parser*);

	// On URL string
	static int onURL(::http_parser*, const char* at, std::size_t length);

	// On header key
	static int onHeaderField(::http_parser*, const char* at, std::size_t length);

	// On header value
	static int onHeaderValue(::http_parser*, const char* at, std::size_t length);

	// Body chunk
	static int onBody(::http_parser*, const char* at, std::size_t length);

	// Resets the last received request (creates a new one)
	void resetRequest();

	// Shutdowns the connection (calls ConnectionLostCallback)
	void shutdown();

	// New data on socket
	void onInput(ev::io& w, int revents);

	// Socket ready to receive data
	void onOutput(ev::io& w, int revents);

	// Waits for new data
	ev::io mInputWatcher;

	// Waits when OS is ready to receive data do send
	ev::io mOutputWatcher;

	// Socket's fd
	int mFD;

	// The request that's being parsed
	std::shared_ptr<Request> mRequest;

	// http-parser structure
	std::unique_ptr<::http_parser> mParser;

	// http-parser settings structure
	::http_parser_settings mParserSettings;

	// Called when a valid request is ready
	InputDataCallback mInputDataCallback;

	// Helper variable for parsing http headers.
	// Last parsed http position was "key". Header is (key:value).
	bool mIsParsingHeaderKey;

	// Helper variable for parsing http headers.
	std::string mLastHeaderKey;
	std::string mLastHeaderValue;

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