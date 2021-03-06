#include <boost/test/unit_test.hpp>

#include "common.hpp"
#include "request.hpp"
#include <iostream>


const std::string TEST_URL = "http://some/url";
const std::string TEST_BODY = "TEST BODY STRING";

BOOST_AUTO_TEST_SUITE(RequestTestSuite)

using namespace everest;

BOOST_AUTO_TEST_CASE(Constructor)
{
	BOOST_CHECK_NO_THROW(Request r(nullptr));
}

BOOST_AUTO_TEST_CASE(Method)
{
	Request r(nullptr);
	r.setMethod(static_cast<unsigned int>(HTTPMethod::HTTP_GET));
	BOOST_CHECK_EQUAL(static_cast<unsigned int>(HTTPMethod::HTTP_GET), r.getMethod());
}

BOOST_AUTO_TEST_CASE(URL)
{
	Request r(nullptr);
	r.appendURL(TEST_URL);
	BOOST_CHECK_EQUAL(TEST_URL, r.getURL());
	r.appendURL(TEST_URL);
	BOOST_CHECK_EQUAL(TEST_URL + TEST_URL, r.getURL());
}

BOOST_AUTO_TEST_CASE(Headers)
{
	Request r(nullptr);

	for (int i = 0; i < 10; ++i) {
		auto x = std::to_string(i);
		r.setHeader(x, x);
	}

	for (int i = 0; i < 10; ++i) {
		auto x = std::to_string(i);
		BOOST_REQUIRE_EQUAL(x, r.getHeader(x));
	}
}

BOOST_AUTO_TEST_CASE(Body)
{
	Request r(nullptr);
	r.appendBody(TEST_BODY);
	BOOST_CHECK_EQUAL(TEST_BODY, r.getBody());

	r.appendBody(TEST_BODY);
	BOOST_CHECK_EQUAL(TEST_BODY + TEST_BODY, r.getBody());
}

BOOST_AUTO_TEST_CASE(ToString)
{
	Request r(nullptr);
	r.setMethod(static_cast<unsigned int>(HTTPMethod::HTTP_GET));
	r.appendURL(TEST_URL);
	for (int i = 0; i < 10; ++i) {
		auto x = std::to_string(i);
		r.setHeader(x, x);
	}
	r.appendBody(TEST_BODY);
	std::cout << r.toString() << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
