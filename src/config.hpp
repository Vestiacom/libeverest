#ifndef EVEREST_CONFIG_HPP_
#define EVEREST_CONFIG_HPP_

#include <string>

namespace everest {

// Configuration options for the Server
struct Config {

	Config(const std::string& url);

	std::string ip;
	unsigned short port;
	double cleanupPeriodSec;
	unsigned maxConnections;
};

} // namespace everest


#endif // EVEREST_CONFIG_HPP_k