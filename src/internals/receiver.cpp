#include "receiver.hpp"
#include "connection.hpp"

#include <ev++.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sstream>


// #include <sys/types.h>
#include <sys/socket.h>

// TODO: Reomove
#include <iostream>
using namespace std;

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
		throw std::runtime_error("ev_loop is null");
	}

	if (mFD < 0) {
		throw std::runtime_error("bad fd");
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
	::shutdown(mFD, ::SHUT_RD);
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
	cout << "onInput" << endl;

	if (EV_ERROR & revents) {
		// Unspecified error
		std::ostringstream msg;
		msg << "Unspecified error in input callback:" <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	// Make this configurable
	ssize_t len = 512;
	char buf[len];

	ssize_t received = ::read(w.fd, buf, len);
	if (received < 0) {
		if (errno == ECONNRESET) {
			// Connection reset by peer.
			shutdown();
			return;
		}

		// TODO: Test throwing in libev watcher
		std::ostringstream msg;
		msg << "Error when reading the Receiver's socket:" <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	// Start / continue parsing. We pass received==0 to signal that EOF has been received
	ssize_t nparsed = http_parser_execute(mParser.get(), &mParserSettings, buf, received);
	if (nparsed != received) {
		throw std::runtime_error("Http parser error");
	}

	if (received == 0) {
		shutdown();
	}
}

void Receiver::resetRequest()
{
	try {
		mRequest = std::make_shared<Request>(mConnection.shared_from_this());
	} catch (std::bad_weak_ptr& e) {
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
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
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
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Receiver::onURL(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);

		std::string url(at, length);
		rec.mRequest->appendURL(url);

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
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
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
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
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
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
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Receiver::onBody(::http_parser* parser, const char* at, size_t length)
{
	try {
		Receiver& rec = *static_cast<Receiver*>(parser->data);
		rec.mRequest->appendBody(std::string(at, length));

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

} // namespace internals
} // namespace everest

