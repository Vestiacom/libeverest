#include <boost/test/unit_test.hpp>

#include "common.hpp"

#include <string>
#include "internals/connection.hpp"
#include <iostream>

BOOST_AUTO_TEST_SUITE(ConnectionTestSuite)

using namespace everest::internals;


BOOST_AUTO_TEST_CASE(BadArgs)
{
	BOOST_CHECK_THROW(Connection a(0, nullptr), std::runtime_error);
	BOOST_CHECK_THROW(Connection a(-1, EV_DEFAULT), std::runtime_error);
	BOOST_CHECK_NO_THROW(Connection a(1, EV_DEFAULT));
}

BOOST_AUTO_TEST_SUITE_END()
