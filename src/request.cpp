#include "request.hpp"

#include <algorithm> // for std::find


namespace everest {

Request::Request(const std::shared_ptr<internals::Connection>& connection)
	: mConnection(connection)
{

}

Request::~Request()
{
}


void Request::appendURL(const std::string& url)
{
	// Long ULR are rare, don't use a string stream
	mURL += url;
}

const std::string& Request::getURL()
{
	return mURL;
}

Request::headers_t::iterator Request::findHeader(const std::string& key)
{
	return std::find_if(mHeaders.begin(), mHeaders.end(), [&key](auto header) {
		return header.first == key;
	});
}

void Request::setHeader(const std::string& key, const std::string& value)
{
	// Is this key duplicated?
	auto it = findHeader(key);
	if (it != mHeaders.end()) {
		it->second = value;
		return;
	}

	mHeaders.emplace_back(key, value);
}

const std::string& Request::getHeader(const std::string& key)
{
	return findHeader(key)->second;
}

void Request::appendBody(const std::string& chunk)
{
	mBodyStream << chunk;
}

std::string Request::getBody()
{
	return mBodyStream.str();
}

} // namespace everest
