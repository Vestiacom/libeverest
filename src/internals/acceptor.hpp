#ifndef EVEREST_INTERNALS_ACCEPTOR_HPP_
#define EVEREST_INTERNALS_ACCEPTOR_HPP_

#include <vector>
#include <ev++.h>
#include <functional>

namespace everest {
namespace internals {

/**
 * Listens on a socket and accepts new connections
 */
struct Acceptor {
	typedef std::function<void(int fd)> NewConnectionCallback;

	Acceptor(const unsigned short port,
	         struct ev_loop* evLoop,
	         const NewConnectionCallback& newConnectionCallback);
	~Acceptor();

	Acceptor(const Acceptor&) = delete;
	Acceptor(Acceptor&&) = delete;
	Acceptor& operator=(const Acceptor&) = delete;

	/**
	 * Starts accepting new connections
	 */
	void start();

	/**
	 * Stops accepting new connections. Can be started again.
	 */
	void stop();

private:
	// Called by the watcher when new connections appear
	void onNewConnection(ev::io& w, int revents);

	// Socket's fd.
	int mFD;

	// Event watcher. Waits for input connections.
	ev::io mWatcher;

	// Callback for new connections
	NewConnectionCallback mNewConnectionCallback;

};

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_ACCEPTOR_HPP_