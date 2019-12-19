#include "request.hpp"

#include <algorithm> // for std::find
#include <random> // for generating UID
#include <algorithm> // for generating UID

namespace everest {

Request::Request(const std::shared_ptr<internals::Connection>& connection)
	: mConnection(connection)
{

}

Request::~Request()
{
}

void Request::setMethod(const unsigned int method)
{
	mMethod = static_cast<HTTPMethod>(method);
}

HTTPMethod Request::getMethod() const
{
	return mMethod;
}

void Request::appendURL(const std::string& url)
{
	// Long ULR are rare, don't use a string stream
	mURL += url;
}

const std::string& Request::getURL() const
{
	return mURL;
}

headers_t::iterator Request::findHeader(const std::string& key)
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

const std::string Request::getHeader(const std::string& key)
{
	auto it = findHeader(key);
	if (it != mHeaders.end()) {
		return it->second;
	}
	return "";
}

std::string Request::toString() const
{
	std::stringstream out;
	out << "Method: " << mMethod << std::endl;
	out << std::endl;

	out << "Headers: " <<  std::endl;
	for (auto header : mHeaders) {
		out << header.first << ": " << header.second << std::endl;
	}
	out << std::endl;


	out << "Body: " <<  std::endl;
	out << getBody();

	return out.str();
}

void Request::appendBody(const std::string& chunk)
{
	mBodyStream << chunk;
}

std::string Request::getBody() const
{
	return mBodyStream.str();
}

int Request::getFD() const
{
	return mConnection->getFD();
}

std::shared_ptr<Response> Request::createResponse()
{
	bool isClosing = getHeader("Connection") == "close";
	return std::make_shared<Response>(mConnection, isClosing);
}

std::string  Request::getUID() const
{
	return mUID;
}

void Request::assignUID()
{
	mUID = getHeader("X-Request-ID");
	if (mUID.empty()) {
		// Create a random made of printable chars

		const char characters[] =
		    "0123456789"
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		    "abcdefghijklmnopqrstuvwxyz";
		std::random_device rd;
		std::uniform_int_distribution<int> dist(0, sizeof(characters) - 1);

		mUID.resize(24);
		std::generate(mUID.begin(), mUID.end(), [&dist, &characters, &rd]() {
			return characters[dist(rd)];
		});
	}
}


} // namespace everest
