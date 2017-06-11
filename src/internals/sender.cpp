#include "sender.hpp"
#include "../response.hpp"

#include <map>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <http_parser.h>

#include <sys/socket.h>

// TODO: Reomove
#include <iostream>
using namespace std;

namespace {

// String statuses tied to the status code.
// Using http-parser macro.
std::map<int, std::string> gStatuses  = {
#define XX(num, name, string) {num, #string},
	HTTP_STATUS_MAP(XX)
#undef XX
};

} // namespace

namespace everest {
namespace internals {


Sender::Sender(int fd, struct ev_loop* evLoop)
	: mOutputWatcher(evLoop),
	  mFD(fd)
{
	if (!evLoop) {
		throw std::runtime_error("ev_loop is null");
	}

	if (mFD < 0) {
		throw std::runtime_error("bad fd");
	}

	mOutputWatcher.set<Sender, &Sender::onOutput>(this);
}

Sender::~Sender()
{
	shutdown();
}

void Sender::start()
{
	if (!mResponses.empty()) {
		mOutputWatcher.start(mFD, ev::WRITE);
	}
}

void Sender::stop()
{
	mOutputWatcher.stop();
}

void Sender::shutdown()
{
	if (mFD < 0) {
		return;
	}

	// No communication with the socket is possible after shutdown
	stop();
	::shutdown(mFD, SHUT_WR);
	mFD = -1;

	while (!mResponses.empty()) {
		mResponses.pop();
	}

	mOutputBuffer.clear();
	mOutputBuffer.shrink_to_fit();
}

bool Sender::isClosed()
{
	return mFD == -1;
}

void Sender::fillBuffer()
{
	auto& response = *mResponses.front();

	// Status line ------------------------------------------------------------
	// For example:
	// HTTP/1.1 200 OK
	// HTTP/1.0 404 Not Found
	// HTTP/1.1 403 Forbidden

	std::string data = "HTTP/1.1 ";
	std::copy(data.begin(), data.end(), std::back_inserter(mOutputBuffer));

	data = std::to_string(response.getStatus()) + " ";
	std::copy(data.begin(), data.end(), std::back_inserter(mOutputBuffer));

	std::string& statusText = gStatuses[response.getStatus()];
	std::copy(statusText.begin(), statusText.end(), std::back_inserter(mOutputBuffer));

	data = "\r\n";
	std::copy(data.begin(), data.end(), std::back_inserter(mOutputBuffer));

	// Headers -----------------------------------------------------------------
	// For example:
	// Accept-Ranges: bytes
	// Content-Length: 44
	// Sender: close

	const auto& headers = response.getHeaders();
	for (const auto& header : headers) {
		std::copy(header.first.begin(), header.first.end(), std::back_inserter(mOutputBuffer));
		mOutputBuffer.push_back(':');
		std::copy(header.second.begin(), header.second.end(), std::back_inserter(mOutputBuffer));
		std::copy(data.begin(), data.end(), std::back_inserter(mOutputBuffer));
	}

	std::copy(data.begin(), data.end(), std::back_inserter(mOutputBuffer));

	// Body -------------------------------------------------------------------
	// Body is already URL encoded
	std::string body = response.getBody();
	std::copy(body.begin(), body.end(), std::back_inserter(mOutputBuffer));

	mResponses.pop();


	// for (auto c : mOutputBuffer) {
	// 	cout << c;
	// }
	// cout << endl;
}

void Sender::onOutput(ev::io& w, int revents)
{
	cout << "onOutput" << endl;

	if (EV_ERROR & revents) {
		// Unspecified error
		std::ostringstream msg;
		msg << "Unspecified error in output callback: " <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
		return;
	}


	// Ensure output buffer has any data to send
	if (mOutputBufferPosition >= mOutputBuffer.size()) {
		// No data to send in mOutputBuffer.

		if (mResponses.empty()) {
			// And there's no more responses to send.
			// Pause sending and free the buffer.
			mOutputBufferPosition = 0;
			mOutputBuffer.clear();
			mOutputWatcher.stop();
			return;
		}

		// Fill mOutputBuffer with the next serialized Response from the queue.
		mOutputBufferPosition = 0;
		mOutputBuffer.resize(0);
		fillBuffer();
	}

	ssize_t n  = ::write(w.fd,
	                     &mOutputBuffer[mOutputBufferPosition],
	                     mOutputBuffer.size() - mOutputBufferPosition);
	if (n >= 0) {
		mOutputBufferPosition += n;
	} else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
		// Neglected errors
	} else {
		if (errno == ECONNRESET) {
			// Connection reset by peer.
			shutdown();
			return;
		}
		std::ostringstream msg;
		msg << "write() failed with: " <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	if (n == 0) {
		shutdown();
	}
}

void Sender::send(const std::shared_ptr<Response>& response)
{
	mResponses.push(response);

	// Ensure sending data is switched on
	mOutputWatcher.start(mFD, ev::WRITE);
}

} // namespace internals
} // namespace everest
