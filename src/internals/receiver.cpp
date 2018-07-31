#include "receiver.hpp"
#include "connection.hpp"
#include "logger.hpp"

#include <ev++.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <vector>


namespace everest {
namespace internals {

Receiver::Receiver(int fd,
                   struct ev_loop* evLoop,
                   Connection& connection,
                   const InputDataCallback& inputDataCallback)
	: mInputWatcher(evLoop),
	  mFD(fd),
	  mConnection(connection),
	  mParser(new ::http_parser),
	  mParserSettings(),
	  mInputDataCallback(inputDataCallback),
	  mIsParsingHeaderKey(false),
	  mLastHeaderKey(""),
	  mLastHeaderValue("")
{
	if (!evLoop) {
		THROW("ev_loop is null");
	}

	if (mFD < 0) {
		THROW("bad fd");
	}

	mInputWatcher.set<Receiver, &Receiver::onInput>(this);

	setupHttpParser();
}

Receiver::~Receiver()
{
	shutdown();
}

void Receiver::setupHttpParser()
{
	mParserSettings.on_url = Receiver::onURL;
	mParserSettings.on_header_field = Receiver::onHeaderField;
	mParserSettings.on_header_value = Receiver::onHeaderValue;
	mParserSettings.on_body = Receiver::onBody;
	mParserSettings.on_message_begin = Receiver::onMessageBegin;
	mParserSettings.on_headers_complete = Receiver::onHeadersComplete;
	mParserSettings.on_message_complete = Receiver::onMessageComplete;

	::http_parser_init(mParser.get(), HTTP_REQUEST);
	mParser->data = this;
}

void Receiver::start()
{
	mInputWatcher.start(mFD, ev::READ);
}

void Receiver::stop()
{
	mInputWatcher.stop();
}

void Receiver::shutdown()
{
	if (mFD < 0) {
		return;
	}

	// No communication with the socket is possible after shutdown
	stop();
	if (-1 == ::shutdown(mFD, SHUT_RD)) {
		LOGD("shutdown() failed with: " <<  std::strerror(errno));
	}
	mFD = -1;

	mLastHeaderKey.clear();
	mLastHeaderValue.clear();
	mParser.reset();
	mRequest.reset();
}

bool Receiver::isClosed()
{
	return mFD == -1;
}

void Receiver::onInput(ev::io& w, int revents)
{
	try {
		if (EV_ERROR & revents) {
			LOGW("Unspecified error in input callback: " <<  std::strerror(errno));
			shutdown();
			return;
		}

		// Make this configurable
		std::vector<char> buf(1024);

		ssize_t received = ::read(w.fd, buf.data(), buf.size());
		if (received < 0) {
			if (errno == ECONNRESET) {
				// Connection reset by peer.
				shutdown();
				return;
			}

			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				LOGD("Neglected error when reading the Receiver's socket:" <<  std::strerror(errno));
				return;
			}

			LOGE("Error when reading the Receiver's socket:" <<  std::strerror(errno));
			shutdown();
			return;
		}

		// Start / continue parsing. We pass received==0 to signal that EOF has been received
		ssize_t nparsed = http_parser_execute(mParser.get(), &mParserSettings, buf.data(), received);
		if (nparsed != received) {
			LOGE("Http parser error. Name: " << http_errno_name(static_cast<http_errno>(mParser->http_errno))
			     << " Description: " << http_errno_description(static_cast<http_errno>(mParser->http_errno)));
			LOGD("Wrong data: " << std::string(buf.begin(), buf.end()));
			shutdown();
			return;
		}

		if (received == 0) {
			shutdown();
		}
	}
	catch (const std::exception& e) {
		LOGE("Unexpected exception: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
}

void Receiver::resetRequest()
{
	try {
		mRequest = std::make_shared<Request>(mConnection.shared_from_this());
	}
	catch (std::bad_weak_ptr& e) {
		// Pathological situation - Connection isn't owned by by shared_ptr.
		// This exception is thrown since in C++17
		mRequest = std::make_shared<Request>(nullptr);
	}
}

int Receiver::onMessageBegin(::http_parser* parser)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);
		rec.resetRequest();

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onMessageComplete(::http_parser* parser)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);
		if (rec.mInputDataCallback) {
			rec.mInputDataCallback(rec.mRequest);
		}
		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onURL(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);

		std::string url(at, length);
		rec.mRequest->appendURL(url);

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onHeaderField(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);

		if (!rec.mIsParsingHeaderKey) {
			// Beginning of a new key

			if (!rec.mLastHeaderKey.empty() && !rec.mLastHeaderValue.empty()) {
				// This isn't the first key:value pair.
				// Add the previous header to the request.
				rec.mRequest->setHeader(rec.mLastHeaderKey, rec.mLastHeaderValue);
			}

			// Set string length to 0.
			// Doesn't release the memory so we won't have to allocate again.
			rec.mLastHeaderKey.clear();

			// Just started receiving key
			rec.mIsParsingHeaderKey = true;
		}

		// Key parsing continuation
		rec.mLastHeaderKey += std::string(at, length);

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onHeaderValue(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);
		if (rec.mIsParsingHeaderKey) {
			// Beginning of the header's value

			// Set string length to 0.
			// Doesn't release the memory so we won't have to allocate again.
			rec.mLastHeaderValue.clear();

			// Just started receiving value
			rec.mIsParsingHeaderKey = false;
		}

		// Value parsing continuation
		rec.mLastHeaderValue += std::string(at, length);

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onHeadersComplete(::http_parser* parser)
{
	try {

		Receiver& rec = *static_cast<Receiver*>(parser->data);

		if (!rec.mLastHeaderKey.empty() && !rec.mLastHeaderValue.empty()) {
			// Set the last key:value header pair (if it exists)
			rec.mRequest->setHeader(rec.mLastHeaderKey, rec.mLastHeaderValue);
		}
		rec.mRequest->setMethod(parser->method);

		rec.mLastHeaderKey.resize(0);
		rec.mLastHeaderValue.resize(0);

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

int Receiver::onBody(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);
		rec.mRequest->appendBody(std::string(at, length));

		// Continue parsing
		return 0;
	}
	catch (const std::exception& e) {
		// Stop parsing the request
		LOGE("Got exception in parsing HTTP: " << e.what());
	}
	catch (...) {
		LOGE("Unexpected exception");
	}
	return -1;
}

} // namespace internals
} // namespace everest

