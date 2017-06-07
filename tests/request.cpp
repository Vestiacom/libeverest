#include <boost/test/unit_test.hpp>

#include "request.hpp"
#include "common.hpp"

const std::string TEST_URL = "http://some/url";
const std::string TEST_BODY = "TEST BODY STRING";

BOOST_AUTO_TEST_SUITE(RequestTestSuite)

using namespace everest;

BOOST_AUTO_TEST_CASE(Constructor)
{
	BOOST_CHECK_NO_THROW(Request r(nullptr));
}

BOOST_AUTO_TEST_CASE(URL)
{
	Request r(nullptr);
	r.appendURL(TEST_URL);
	BOOST_CHECK_EQUAL(TEST_URL, r.getURL());
	r.appendURL(TEST_URL);
	BOOST_CHECK_EQUAL(TEST_URL+TEST_URL, r.getURL());
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

BOOST_AUTO_TEST_SUITE_END()
