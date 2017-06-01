#include <boost/test/unit_test.hpp>

#include <string>
#include "internals/acceptor.hpp"
#include <iostream>

#include <thread>
#include "common.hpp"

BOOST_AUTO_TEST_SUITE(AcceptorTestSuite)

using namespace everest::internals;

const unsigned short TEST_PORT = 6000;

BOOST_AUTO_TEST_CASE(BadArgs)
{
	// Port 53 is most likely taken
	BOOST_CHECK_THROW(Acceptor a(53, EV_DEFAULT, [](int) {}), std::runtime_error);
	BOOST_CHECK_THROW(Acceptor a(TEST_PORT, nullptr, [](int) {}), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Accept)
{
	bool isCalled = false;

	struct ev_loop* loop = EV_DEFAULT;

	Acceptor a(TEST_PORT, loop, [&](int) {
		ev_break(loop, EVBREAK_ALL);
		isCalled = true;
	});

	a.start();

	runParallel("wget localhost:6000 -T 1 --tries=1 -q");

	ev_run(loop, 0);

	BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
