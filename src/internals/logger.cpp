#include "logger.hpp"

namespace everest {

namespace internals {

LogCallback thread_local gLogCallback = [](const everest::LogLevel level, const std::string& msg)
{
	std::cout << "[" << everest::toString(level) << "]\t" << msg << std::endl;
	std::cout.flush();
};

} // namespace internals

std::string toString(const LogLevel logLevel)
{
	switch (logLevel) {
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::WARN:
		return "WARN ";
	case LogLevel::INFO:
		return "INFO ";
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::TRACE:
		return "TRACE";
	default:
		return "UNKNOWN";
	}
}

void setLogger(const LogCallback& logCallback)
{
	internals::gLogCallback = logCallback;
}


} // namespace everest