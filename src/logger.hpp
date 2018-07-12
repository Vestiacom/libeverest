#ifndef EVEREST_LOGGER_HPP_
#define EVEREST_LOGGER_HPP_

#include "types.hpp"

namespace everest {

std::string toString(const LogLevel logLevel);

void setLogger(const LogCallback& logCallback);

} // namespace internals

#endif // EVEREST_LOGGER_HPP_
