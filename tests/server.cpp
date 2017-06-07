#include <boost/test/unit_test.hpp>

#include <string>
#include "server.hpp"
#include <iostream>

#include <thread>
#include "common.hpp"

BOOST_AUTO_TEST_SUITE(ServerTestSuite)

using namespace everest;

const unsigned short TEST_PORT = 6000;
const std::string TEST_URL = "/test";
const std::string TEST_BODY = "TEST BODY";
const std::string TEST_HEADER_KEY = "A";
const std::string TEST_HEADER_VALUE = "B";

// BOOST_AUTO_TEST_CASE(BadArgs)
// {
// 	// Port 53 is most likely taken
// 	BOOST_CHECK_THROW(Acceptor a(53, EV_DEFAULT, [](int) {}), std::runtime_error);
// 	BOOST_CHECK_THROW(Acceptor a(TEST_PORT, nullptr, [](int) {}), std::runtime_error);
// }

BOOST_AUTO_TEST_CASE(GET)
{
	bool isOK = false;

	struct ev_loop* loop = EV_DEFAULT;

	Server s(TEST_PORT, loop);
	s.endpoint(TEST_URL, [&](const std::shared_ptr<Request>& r) {
		isOK = r->getURL() == TEST_URL
		       && r->getHeader(TEST_HEADER_KEY) == TEST_HEADER_VALUE;
		ev_break(loop, EVBREAK_ALL);
	});
	s.start();

	runParallel("wget -T 1 --tries=1 -q --header=\'" + TEST_HEADER_KEY + ":" + TEST_HEADER_VALUE
	            + "\' localhost:" + std::to_string(TEST_PORT)
	            + TEST_URL);

	ev_run(loop, 0);

	BOOST_CHECK(isOK);
}

BOOST_AUTO_TEST_CASE(POST)
{
	bool isOK = false;

	struct ev_loop* loop = EV_DEFAULT;

	Server s(TEST_PORT, loop);
	s.endpoint(TEST_URL, [&](const std::shared_ptr<Request>& r) {
		isOK = r->getURL() == TEST_URL
		       && r->getHeader(TEST_HEADER_KEY) == TEST_HEADER_VALUE
		       && r->getBody() == TEST_BODY;
		ev_break(loop, EVBREAK_ALL);
	});
	s.start();

	runParallel("wget -T 1 --tries=1 -q --post-data=\'" + TEST_BODY
	            + "\' --header=\'" + TEST_HEADER_KEY + ":" + TEST_HEADER_VALUE
	            + "\' localhost:" + std::to_string(TEST_PORT)
	            + TEST_URL);

	ev_run(loop, 0);

	BOOST_CHECK(isOK);
}

BOOST_AUTO_TEST_SUITE_END()
