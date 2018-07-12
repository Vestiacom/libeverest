#include "logger.hpp"
#include "../log.hpp"

namespace everest {
namespace internals {

LogCallback thread_local gLogCallback = [](const everest::LogLevel level, const std::string& msg)
{
	std::cout << "[" << everest::toString(level) << "]\t" << msg << std::endl;
	std::cout.flush();
};

} // namespace internals
} // namespace everest