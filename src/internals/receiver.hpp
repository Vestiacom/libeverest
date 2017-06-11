#ifndef EVEREST_INTERNALS_RECEIVER_HPP_
#define EVEREST_INTERNALS_RECEIVER_HPP_

// #include "../request.hpp"

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

class Connection;

/**
 * Receives data from the socket and parses HTTP requests.
 * Requests are passed down to the callback.
 * There's only one Receiver per Connection.
 */
struct Receiver {
	typedef std::function<void(const std::shared_ptr<Request>& request)> InputDataCallback;

	Receiver(int fd,
	         struct ev_loop* evLoop,
	         Connection& connection,
	         const InputDataCallback& inputDataCallback);
	~Receiver();

	Receiver(const Receiver&) = delete;
	Receiver(Receiver&&) = delete;
	Receiver& operator=(const Receiver&) = delete;

	/**
	 * Starts receiving
	 */
	void start();

	/**
	 * Stops receiving
	 */
	void stop();

	/**
	 * @return is this connection closed
	 */
	bool isClosed();

	/**
	 * Shutdowns the connection
	 */
	void shutdown();

private:
	// Http proxy initialization
	void setupHttpParser();

	// http-parser: Headers parsing finished
	static int onHeadersComplete(::http_parser*);

	// http-parser: New message parsing started
	static int onMessageBegin(::http_parser*);

	// http-parser: Message is parsed and request is ready
	static int onMessageComplete(::http_parser*);

	// http-parser: On URL string
	static int onURL(::http_parser*, const char* at, std::size_t length);

	// http-parser: On header key
	static int onHeaderField(::http_parser*, const char* at, std::size_t length);

	// http-parser: On header value
	static int onHeaderValue(::http_parser*, const char* at, std::size_t length);

	// http-parser: Body chunk
	static int onBody(::http_parser*, const char* at, std::size_t length);

	// Resets the last received request (creates a new one)
	void resetRequest();

	// New data on socket
	void onInput(ev::io& w, int revents);

	// Waits for new data
	ev::io mInputWatcher;

	// Socket's fd
	int mFD;

	// "Parent" connection.
	// It will always exist for a given Receiver.
	// It can't be a shared_ptr to avoid dependency loops.
	Connection& mConnection;

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
};

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_RECEIVER_HPP_