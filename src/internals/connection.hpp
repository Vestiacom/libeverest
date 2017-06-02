#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include <vector>
#include <functional> // std::function
#include <memory> // std::unique_ptr
#include <ev++.h>
#include <http_parser.h>

namespace everest {
namespace internals {

/**
 * Wraps together all data used by one connection and HTTP parsing for this connection.
 */
struct Connection {
	typedef std::function<void(void)> InputDataCallback;

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

	// Message is parsed and request is ready
	static int onMessageComplete(http_parser*);

	// On URL string
	static int onURL(http_parser*, const char* at, size_t length);

	// On header key
	static int onHeaderField(http_parser*, const char* at, size_t length);

	// On header value
	static int onHeaderValue(http_parser*, const char* at, size_t length);

	// Body chunk
	static int onBody(http_parser*, const char* at, size_t length);

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

	// Input buffer
	std::vector<char> mIN;

	// Output buffer
	std::vector<char> mOUT;

	// http-parser structure
	std::unique_ptr<http_parser> mParser;

	// Called when a valid request is ready
	InputDataCallback mInputDataCallback;



};

} // namespace internals

} // namespace everest

#endif // EVEREST_INTERNALS_CONNECTION_HPP_