#include <boost/test/unit_test.hpp>

#include "common.hpp"

#include <string>
#include <iostream>
#include <thread>

#include "internals/logger.hpp"
#include "types.hpp"


BOOST_AUTO_TEST_SUITE(LoggerTestSuite)

using namespace everest::internals;

const unsigned short TEST_PORT = 6000;
const unsigned short TEST_HOST = 6000;

BOOST_AUTO_TEST_CASE(SetLogger)
{
	everest::setLogger([](const everest::LogLevel level, const std::string & msg) {
		std::cout << "LOL, I got some logs! " << everest::toString(level) << msg << std::endl;
		std::cout.flush();
	});
}


BOOST_AUTO_TEST_SUITE_END()
