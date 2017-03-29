#ifndef EVEREST_SERVER_HPP_
#define EVEREST_SERVER_HPP_

#include <ev++.h>
#include <functional>


namespace everest {

struct Server {

	// typedef std::function<std::string(const std::string&)> Callback;

	Server(const unsigned short port, struct ev_loop* evLoop);
	~Server();

	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;

	/**
	 * Starts serving requests using the ev loop
	 *
	 * @param ev_loop ev loop
	 */
	void start();

	/**
	 * Stops serving requests.
	 */
	void stop();

private:

	// 
	// std::unordered_map<int, ev::io> mConnectionsIO;

	// // Output buffers
	// // Calls to write() put data here and WRITE events take it and send via socket
	// std::unordered_map<int, std::vector<char>> mBuffers;


	// Event loop
	struct ev_loop* mEvLoop;

	// Watcher for the incoming data
	ev::io mIO;

	int mFD;

	// Creates the input socket
	void createListeningSocket(const unsigned short port);

	// // Called when new connection is established.
	// void onNewConnection(ev::io& w, int revents);

	// // Called when data arrived to the socket
	// void onInput(ev::io& w, int revents);

	// Generic callback for libev. 
	// Used to write and read data from the connected socket
	void onEvent(ev::io& w, int revents);

};

} // namespace everest

#endif // EVEREST_SERVER_HPP_