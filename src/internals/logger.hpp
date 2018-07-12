#ifndef EVEREST_INTERNALS_LOGGER_HPP_
#define EVEREST_INTERNALS_LOGGER_HPP_

#include <sstream>
#include <iostream>
#include "../types.hpp"

namespace everest {

namespace internals {

extern thread_local LogCallback gLogCallback;

/// Generic logging macro. Needs mLogger callback to be present.
#define LOG(LEVEL, MESSAGE)          \
    do {                             \
        std::ostringstream msg__;    \
        msg__ << MESSAGE << " [" << __FILENAME__ << ":" << __LINE__ << "]" ;   \
        if (everest::internals::gLogCallback){                    \
        	everest::internals::gLogCallback(LEVEL, msg__.str());} \
        } while (0)

#define THROW(MESSAGE)          \
    do {                             \
        std::ostringstream msg__;    \
        msg__ << "libeverest: " << MESSAGE << " [" << __FILENAME__ << ":" << __LINE__ << "]" ;   \
        if (everest::internals::gLogCallback){                    \
        	everest::internals::gLogCallback(everest::LogLevel::ERROR, msg__.str());} \
        throw std::runtime_error(msg__.str()); \
        } while (0)

#define LOGE(MESSAGE) LOG(everest::LogLevel::ERROR, MESSAGE)
#define LOGW(MESSAGE) LOG(everest::LogLevel::WARN, MESSAGE)
#define LOGI(MESSAGE) LOG(everest::LogLevel::INFO, MESSAGE)


#if !defined(NDEBUG)

#define LOGD(MESSAGE) LOG(everest::LogLevel::DEBUG, MESSAGE)
#define LOGT(MESSAGE) LOG(everest::LogLevel::TRACE, MESSAGE)

#else

#define LOGD(MESSAGE) do {} while (0)
#define LOGT(MESSAGE) do {} while (0)

#endif

} // everest
} // namespace internals

#endif // EVEREST_INTERNALS_LOGGER_HPP_
