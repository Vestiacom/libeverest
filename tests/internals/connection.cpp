#include <boost/test/unit_test.hpp>

#include "common.hpp"

#include <string>
#include "internals/connection.hpp"
#include "request.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(ConnectionTestSuite)

using namespace everest;
using namespace everest::internals;

const std::string TEST_URL = "/test";
const std::string TEST_BODY = "body";
const std::string TEST_HEADER_KEY = "key";
const std::string TEST_HEADER_VALUE = "value";
const std::string TEST_DATA = "GET " + TEST_URL + " HTTP/1.1\r\n" + TEST_HEADER_KEY + ":" + TEST_HEADER_VALUE + "\r\n\
Content-Length: 4\r\n\
\r\n" + TEST_BODY;


auto TEST_CB = [](const std::shared_ptr<Request>&) {};

#include <iostream>
using namespace std;

BOOST_AUTO_TEST_CASE(BadArgs)
{
	BOOST_CHECK_THROW(Connection a(10, nullptr, TEST_CB), std::runtime_error);
	BOOST_CHECK_THROW(Connection a(-1, EV_DEFAULT, TEST_CB), std::runtime_error);
	// BOOST_CHECK_NO_THROW(Connection a(1, EV_DEFAULT, TEST_CB));
}

BOOST_AUTO_TEST_CASE(ReceiveData)
{
	int fd[2];
	BOOST_CHECK(pipe(fd) != 1);

	bool isOK = false;
	struct ev_loop* loop = EV_DEFAULT;
	Connection c(fd[0], loop, [&](const std::shared_ptr<Request>& r) {
		isOK = r->getBody() == TEST_BODY
		       && r->getURL() == TEST_URL
		       && r->getHeader(TEST_HEADER_KEY) == TEST_HEADER_VALUE;
		ev_break(loop, EVBREAK_ALL);
	});
	c.start();

	BOOST_CHECK(-1 != ::write(fd[1], TEST_DATA.c_str(), TEST_DATA.size()));

	ev_run(loop, 0);

	BOOST_CHECK(isOK);
}

BOOST_AUTO_TEST_SUITE_END()
