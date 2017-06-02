#include <boost/test/unit_test.hpp>

#include "common.hpp"

#include <string>
#include "internals/connection.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(ConnectionTestSuite)

using namespace everest::internals;

const std::string TEST_DATA = "TEST DATA TO SEND";

BOOST_AUTO_TEST_CASE(BadArgs)
{
	BOOST_CHECK_THROW(Connection a(10, nullptr, []() {}), std::runtime_error);
	BOOST_CHECK_THROW(Connection a(-1, EV_DEFAULT, []() {}), std::runtime_error);
	BOOST_CHECK_NO_THROW(Connection a(1, EV_DEFAULT, []() {}));
}

BOOST_AUTO_TEST_CASE(ReceiveData)
{
	int fd[2];
	BOOST_CHECK(pipe(fd) != 1);

	bool isCalled = false;
	struct ev_loop* loop = EV_DEFAULT;
	Connection c(fd[0], loop, [&]() {
		ev_break(loop, EVBREAK_ALL);
		isCalled = true;
	});
	c.start();


	BOOST_CHECK(-1 != ::write(fd[1], TEST_DATA.c_str(), TEST_DATA.size()));

	ev_run(loop, 0);

	BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
