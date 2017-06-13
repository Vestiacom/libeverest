#include <boost/test/unit_test.hpp>

#include "common.hpp"

#include <string>
#include <iostream>
#include <thread>

#include "internals/url.hpp"


BOOST_AUTO_TEST_SUITE(URLTestSuite)

using namespace everest::internals;

const unsigned short TEST_PORT = 6000;
const unsigned short TEST_HOST = 6000;

BOOST_AUTO_TEST_CASE(BadArgs)
{
	// Port 53 is most likely taken
	BOOST_CHECK_THROW(URL u("localhost::"), std::runtime_error);
	BOOST_CHECK_THROW(URL u("localhost:"), std::runtime_error);
	BOOST_CHECK_THROW(URL u("0.0.0.0:"), std::runtime_error);
	BOOST_CHECK_THROW(URL u(":1500"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(GoodUrls)
{
	BOOST_CHECK_NO_THROW(URL u("localhost:1500"));
	BOOST_CHECK_NO_THROW(URL u("0.0.0.0:1500"));
}

BOOST_AUTO_TEST_SUITE_END()
