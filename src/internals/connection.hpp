#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include <vector>
#include <ev++.h>

namespace everest {
namespace internals {

/**
 * Wraps together all data used by one connection and HTTP parsing for this connection.
 */
struct Connection {
	Connection(int fd, struct ev_loop* evLoop);
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


};

} // namespace internals

} // namespace everest

#endif // EVEREST_INTERNALS_CONNECTION_HPP_