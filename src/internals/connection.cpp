#include "connection.hpp"

#include <vector>
#include <ev++.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

// #include <sys/types.h>
// #include <sys/socket.h>



// TODO: Reomove
#include <iostream>
using namespace std;


namespace everest {
namespace internals {


Connection::Connection(int fd,
                       struct ev_loop* evLoop,
                       const InputDataCallback& inputDataCallback)
	: mInputWatcher(evLoop),
	  mOutputWatcher(evLoop),
	  mFD(fd),
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

	mInputWatcher.set<Connection, &Connection::onInput>(this);
	mOutputWatcher.set<Connection, &Connection::onOutput>(this);

	setupHttpProxy();
}


Connection::~Connection()
{
	mInputWatcher.stop();
	mOutputWatcher.stop();

	::close(mFD);
}

void Connection::setupHttpProxy()
{
	mParserSettings.on_url = Connection::onURL;
	mParserSettings.on_header_field = Connection::onHeaderField;
	mParserSettings.on_header_value = Connection::onHeaderValue;
	mParserSettings.on_body = Connection::onBody;
	mParserSettings.on_message_begin = Connection::onMessageBegin;
	mParserSettings.on_headers_complete = Connection::onHeadersComplete;
	mParserSettings.on_message_complete = Connection::onMessageComplete;

	::http_parser_init(mParser.get(), HTTP_REQUEST);
	mParser->data = this;
}

void Connection::start()
{
	mInputWatcher.start(mFD, ev::READ);
	mOutputWatcher.start(mFD, ev::WRITE);
}

void Connection::stop()
{
	mInputWatcher.stop();
	mOutputWatcher.stop();
}

void Connection::onInput(ev::io& w, int revents)
{
	if (EV_ERROR & revents) {
		// Unspecified error
		std::ostringstream msg;
		msg << "Error when reading the Connection's socket:" <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	// Make this configurable
	ssize_t len = 512;
	char buf[len];

	ssize_t recved = ::read(w.fd, buf, len);
	if (recved < 0) {
		// TODO: Test throwing in libev watcher
		std::ostringstream msg;
		msg << "Error when reading the Connection's socket:" <<  std::strerror(errno);
		throw std::runtime_error(msg.str());
	}

	// Start / continue parsing. We pass recved==0 to signal that EOF has been received
	ssize_t nparsed = http_parser_execute(mParser.get(), &mParserSettings, buf, recved);
	if (nparsed != recved) {
		throw std::runtime_error("Http parser error");
	}
}

void Connection::onOutput(ev::io& w, int revents)
{
	if (EV_ERROR & revents) {
		// Unspecified error
		return;
	}

	(void) w;
	// TODO: Implement async data sending
}

void Connection::send()
{

}

int Connection::onMessageBegin(::http_parser* parser)
{
	cout << "onMessageBegin" << endl;

	try {
		Connection& conn = *static_cast<Connection*>(parser->data);
		conn.mRequest = std::make_shared<Request>();

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onMessageComplete(::http_parser* parser)
{
	cout << "onMessageComplete" << endl;
	try {
		Connection& conn = *static_cast<Connection*>(parser->data);
		if (conn.mInputDataCallback) {
			conn.mInputDataCallback(conn.mRequest);
		}

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onURL(::http_parser* parser, const char* at, size_t length)
{
	cout << "onURL" << endl;
	try {
		Connection& conn = *static_cast<Connection*>(parser->data);

		std::string url(at, length);
		conn.mRequest->appendURL(url);

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onHeaderField(::http_parser* parser, const char* at, size_t length)
{
	cout << "onHeaderField" << endl;
	try {
		Connection& conn = *static_cast<Connection*>(parser->data);

		if (!conn.mIsParsingHeaderKey) {
			// Beginning of a new key

			if (!conn.mLastHeaderKey.empty() && !conn.mLastHeaderValue.empty()) {
				// This isn't the first key:value pair.
				// Add the previous header to the request.
				conn.mRequest->setHeader(conn.mLastHeaderKey, conn.mLastHeaderValue);
			}

			// Set string length to 0.
			// Doesn't release the memory so we won't have to allocate again.
			conn.mLastHeaderKey.clear();

			// Just started receiving key
			conn.mIsParsingHeaderKey = true;
		}

		// Key parsing continuation
		conn.mLastHeaderKey += std::string(at, length);

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onHeaderValue(::http_parser* parser, const char* at, size_t length)
{
	cout << "onHeaderValue" << endl;
	try {
		Connection& conn = *static_cast<Connection*>(parser->data);
		if (conn.mIsParsingHeaderKey) {
			// Beginning of the header's value

			// Set string length to 0.
			// Doesn't release the memory so we won't have to allocate again.
			conn.mLastHeaderValue.clear();

			// Just started receiving value
			conn.mIsParsingHeaderKey = false;
		}

		// Value parsing continuation
		conn.mLastHeaderValue += std::string(at, length);

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onHeadersComplete(::http_parser* parser)
{
	cout << "onHeadersComplete" << endl;
	try {

		Connection& conn = *static_cast<Connection*>(parser->data);

		// Set the last key:value header pair
		conn.mRequest->setHeader(conn.mLastHeaderKey, conn.mLastHeaderValue);

		conn.mLastHeaderKey.resize(0);
		conn.mLastHeaderValue.resize(0);

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}

int Connection::onBody(::http_parser* parser, const char* at, size_t length)
{
	cout << "onBody" << endl;
	try {
		Connection& conn = *static_cast<Connection*>(parser->data);
		conn.mRequest->appendBody(std::string(at, length));

		// Continue parsing
		return 0;
	} catch (const std::exception&) {
		// Stop parsing the request
		return -1;
	}
}



} // namespace internals
} // namespace everest

