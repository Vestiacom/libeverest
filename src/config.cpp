#include "config.hpp"
#include "internals/url.hpp"

namespace everest {

Config::Config(const std::string& url)
	: cleanupPeriodSec(20),
	  maxConnections(1020),
	  maxRequestTimeSec(60)
{
	internals::URL u(url);
	ip = u.hostname;
	port = u.port;
}

} // namespace everest
