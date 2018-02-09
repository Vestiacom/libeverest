#ifndef EVEREST_RESPONSE_HPP_
#define EVEREST_RESPONSE_HPP_

#include <map>
#include <list>
#include <string>
#include <sstream>
#include <memory>

#include "internals/connection.hpp"
#include "internals/common.hpp"

#include "types.hpp"

namespace everest {

namespace internals {
struct Connection;
}

/**
 * Keeps all data of an outgoing HTTP response.
 * Handles data appending.
 *
 * If there are many Response objects per one connection you have to call send:
 * - for each of them
 * - in the order they came to your callback
 */
struct EVEREST_API Response: std::enable_shared_from_this<Response> {

	// TODO: Make constructor protected
	Response(const std::shared_ptr<internals::Connection>& connection, const bool isClosing = false);
	~Response();

	Response(const Response&) = delete;
	Response(Response&&) = delete;
	Response& operator=(const Response&) = delete;

	void setStatus(const unsigned short status);
	unsigned short getStatus();

	void setHeader(const std::string& key, const std::string& value);
	const std::string getHeader(const std::string& key);
	const headers_t& getHeaders();

	void appendBody(const std::string& chunk);
	std::string getBody();

	void send();

	void setClosing(bool isClosing = true);

	bool isClosing();
private:
	headers_t::iterator findHeader(const std::string& key);
	headers_t mHeaders;

	unsigned short mStatus;
	std::stringstream mBodyStream;

	std::shared_ptr<internals::Connection> mConnection;

	// This is the last, closing response.
	// After it's sent the connection will be closed.
	bool mIsClosing;
};

} // namespace everest

#endif // EVEREST_RESPONSE_HPP_