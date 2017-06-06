#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include "request.hpp"

#include <vector>
#include <functional>
#include <memory>
#include <ev++.h>
#include <http_parser.h>

namespace everest {
namespace internals {

/**
 * Wraps together all data used by one connection and HTTP parsing for this connection.
 */
struct Connection {
	typedef std::function<void(const std::shared_ptr<Request>&)> InputDataCallback;

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
	void send();

private:

	// Http proxy initialization
	void setupHttpProxy();

	// Headers parsing finished
	static int onHeadersComplete(::http_parser*);

	// New message parsing started
	static int onMessageBegin(::http_parser*);

	// Message is parsed and request is ready
	static int onMessageComplete(::http_parser*);

	// On URL string
	static int onURL(::http_parser*, const char* at, size_t length);

	// On header key
	static int onHeaderField(::http_parser*, const char* at, size_t length);

	// On header value
	static int onHeaderValue(::http_parser*, const char* at, size_t length);

	// Body chunk
	static int onBody(::http_parser*, const char* at, size_t length);

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
};

} // namespace internals

} // namespace everest

#endif // EVEREST_INTERNALS_CONNECTION_HPP_