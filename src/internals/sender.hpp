#ifndef EVEREST_INTERNALS_SENDER_HPP_
#define EVEREST_INTERNALS_SENDER_HPP_

#include <queue>
#include <vector>
#include <memory>
#include <ev++.h>

namespace everest {

struct Response;


namespace internals {

/**
 * Sends out HTTP data via the passed socket
 * There's only one Sender per Connection.
 */
struct Sender {

	Sender(int fd, struct ev_loop* evLoop);
	~Sender();

	Sender(const Sender&) = delete;
	Sender(Sender&&) = delete;
	Sender& operator=(const Sender&) = delete;

	/**
	 * Starts sending data
	 */
	void start();

	/**
	 * Stops sending data
	 */
	void stop();

	/**
	 * Write data to the output buffer.
	 * Will not block.
	 */
	void send(const std::shared_ptr<Response>& response);

	/**
	 * @return is this connection closed
	 */
	bool isClosed();

	/**
	 * Shutdowns the connection
	 */
	void shutdown();

private:
	// Pop the oldest Response and serialize it to the output buffer
	void fillBuffer();

	// Socket ready to receive data
	void onOutput(ev::io& w, int revents);

	// Waits when OS is ready to receive data do send
	ev::io mOutputWatcher;

	// Socket's fd
	int mFD;

	// When Response is send gets stored here.
	// Responses are send in order, one by one, in the onOutput callback.
	// The front (oldest) Response is serialized into the mOutputBuffer
	std::queue<std::shared_ptr<Response>> mResponses;

	// Buffer with the latest response to send
	std::vector<char> mOutputBuffer;

	// Position in the buffer.
	// Some data might have been send,
	// this is the position of the first unsent byte in the mOutputBuffer.
	std::size_t mOutputBufferPosition;


	// Flow control variable.
	// Does the response that is being send have the Connection: close header?
	bool mIsClosing;
};

} // namespace internals
} // namespace everest

#endif // EVEREST_INTERNALS_SENDER_HPP_