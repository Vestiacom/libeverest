#include "sender.hpp"
#include "logger.hpp"
#include "../response.hpp"

#include <map>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <http_parser.h>

#include <sys/socket.h>

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
	  mFD(fd),
	  mIsClosing(false)
{
	if (!evLoop) {
		THROW("ev_loop is null");
	}

	if (mFD < 0) {
		THROW("bad fd");
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

	if (-1 == ::shutdown(mFD, SHUT_WR)) {
		LOGD("shutdown() failed with: " <<  std::strerror(errno));
	}
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
	LOGD("Serializing response...");

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

	// Should this be the last message?
	mIsClosing = response.isClosing();

	mResponses.pop();

	LOGT(std::string(mOutputBuffer.data(), mOutputBuffer.size()));
}

void Sender::onOutput(ev::io& w, int revents)
{
	LOGD("Ready to send data on fd: " <<  mFD);

	if (EV_ERROR & revents) {
		LOGE("Unspecified error in output callback: " <<  std::strerror(errno));
		shutdown();
		return;
	}


	// Ensure output buffer has any data to send
	if (mOutputBufferPosition >= mOutputBuffer.size()) {
		// No data to send in mOutputBuffer.

		if (mIsClosing) {
			// And it was the last message in this connection
			shutdown();
			return;
		}


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

	LOGD("Sending response chunk...");
	ssize_t n  = ::write(w.fd,
	                     &mOutputBuffer[mOutputBufferPosition],
	                     mOutputBuffer.size() - mOutputBufferPosition);
	LOGD("Sent: " << n << "/" << mOutputBuffer.size() - mOutputBufferPosition << " bytes");

	if (n >= 0) {
		mOutputBufferPosition += n;
	}
	else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
		// Neglected errors
	}
	else {
		if (errno == ECONNRESET) {
			// Connection reset by peer.
			shutdown();
			return;
		}
		LOGD("write() failed with: " <<  std::strerror(errno));
		shutdown();
		return;
	}

	if (n == 0) {
		shutdown();
	}
}

void Sender::send(const std::shared_ptr<Response>& response)
{
	// Can't send if disconnected
	if (isClosed()) {
		THROW("connection closed, can't send response");
	}

	mResponses.push(response);

	// Ensure sending data is switched on
	mOutputWatcher.start(mFD, ev::WRITE);
}

} // namespace internals
} // namespace everest
