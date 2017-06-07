#include <boost/test/unit_test.hpp>

#include "response.hpp"
#include "common.hpp"

const std::string TEST_BODY = "TEST BODY STRING";

BOOST_AUTO_TEST_SUITE(ResponseTestSuite)

using namespace everest;

BOOST_AUTO_TEST_CASE(Constructor)
{
	BOOST_CHECK_NO_THROW(Response r(nullptr));
}

BOOST_AUTO_TEST_CASE(Status)
{
	Response r(nullptr);
	r.setStatus(200);
	BOOST_CHECK_EQUAL(200, r.getStatus());
}

BOOST_AUTO_TEST_CASE(Headers)
{
	Response r(nullptr);

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
	Response r(nullptr);
	r.appendBody(TEST_BODY);
	BOOST_CHECK_EQUAL(TEST_BODY, r.getBody());

	r.appendBody(TEST_BODY);
	BOOST_CHECK_EQUAL(TEST_BODY + TEST_BODY, r.getBody());
}

BOOST_AUTO_TEST_SUITE_END()
