#ifndef EVEREST_INTERNALS_CONNECTION_HPP_
#define EVEREST_INTERNALS_CONNECTION_HPP_

#include <vector>
#include <ev++.h>

namespace everest {
namespace internals {

/**
 * Wraps together all data used by one connection
 */
struct Connection {
	Connection(int fd);

	Connection(const Connection&) = delete;
	Connection(Connection&&) = delete;
	Connection& operator=(const Connection&) = delete;

	// Event watcher. Waits for input or output
	ev::io mWatcher;

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