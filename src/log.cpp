#include "log.hpp"
#include "internals/logger.hpp"

namespace everest {

std::string toString(const LogLevel logLevel)
{
	switch (logLevel) {
	case LogLevel::EVEREST_LOG_LEVEL_ERROR:
		return "ERROR";
	case LogLevel::EVEREST_LOG_LEVEL_WARN:
		return "WARN ";
	case LogLevel::EVEREST_LOG_LEVEL_INFO:
		return "INFO ";
	case LogLevel::EVEREST_LOG_LEVEL_DEBUG:
		return "DEBUG";
	case LogLevel::EVEREST_LOG_LEVEL_TRACE:
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