#ifndef EVEREST_INTERNALS_REQUEST_HPP_
#define EVEREST_INTERNALS_REQUEST_HPP_

#include <list>
#include <string>
#include <sstream>

namespace everest {
namespace internals {

/**
 * Keeps all data of an incoming HTTP request.
 * Handles data appending to ease HTTP parsing.
 */
struct Request {
	Request();
	~Request();

	Request(const Request&) = delete;
	Request(Request&&) = delete;
	Request& operator=(const Request&) = delete;

	void setURL(const std::string& url);
	const std::string& getURL();

	void setHeader(const std::string& key, const std::string& value);
	const std::string& getHeader(const std::string& key);

	void appendBody(const std::string& chunk);
	std::string getBody();


private:
	typedef std::list<std::pair<std::string, std::string>> headers_t;
	headers_t::iterator findHeader(const std::string& key);
	headers_t mHeaders;

	std::string mURL;

	std::stringstream mBodyStream;
};

} // namespace internals

} // namespace everest

#endif // EVEREST_INTERNALS_REQUEST_HPP_