#ifndef EVEREST_INTERNALS_REQUEST_HPP_
#define EVEREST_INTERNALS_REQUEST_HPP_

#include <list>
#include <string>
#include <sstream>
#include <memory>

// #include "internals/connection.hpp"
#include "response.hpp"
#include "types.hpp"

namespace everest {
class Response;

namespace internals {
class Connection;
}

/**
 * Keeps all data of an incoming HTTP request.
 * Handles data appending to ease HTTP parsing.
 */
struct Request {
	friend class internals::Connection;


	Request(const std::shared_ptr<internals::Connection>& connection);
	~Request();

	Request(const Request&) = delete;
	Request(Request&&) = delete;
	Request& operator=(const Request&) = delete;


	// Access data
	HTTPMethod getMethod();
	const std::string& getURL();
	const std::string& getHeader(const std::string& key);
	std::string getBody();

	// Respond
	std::shared_ptr<Response> createResponse();

private:

	// Methods called only by the Connection, when parsing Request
	void setMethod(const unsigned int method);
	void appendURL(const std::string& url);
	void setHeader(const std::string& key, const std::string& value);
	void appendBody(const std::string& chunk);

	// Helper method
	headers_t::iterator findHeader(const std::string& key);

	// Request's data
	headers_t mHeaders;
	HTTPMethod mMethod;
	std::string mURL;
	std::stringstream mBodyStream;

	// Reference to the connection. Needed to create the Response object.
	// We don't keep the Response here to allow deleting Response before Request.
	std::shared_ptr<internals::Connection> mConnection;
};

} // namespace everest

#endif // EVEREST_INTERNALS_REQUEST_HPP_