#ifndef EVEREST_TYPES_HPP_
#define EVEREST_TYPES_HPP_

#include <http_parser.h>
#include <list>
#include <functional>

namespace everest {

/// Log level used in the logging callback
enum class LogLevel : int {
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
};

typedef std::function<void(LogLevel, const std::string& message)> LogCallback;

typedef ::http_method HTTPMethod;

typedef std::list<std::pair<std::string, std::string>> headers_t;

} // namespace everest

#endif // EVEREST_TYPES_HPP_