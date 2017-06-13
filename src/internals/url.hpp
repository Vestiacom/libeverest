#ifndef EVEREST_INTERNAL_URL_HPP_
#define EVEREST_INTERNAL_URL_HPP_

#include <string>

namespace everest {
namespace internals {

/**
 * Helper class for parsing connection URL.
 */
struct URL {
	URL(const std::string& url);

	std::string host;
	unsigned short port;
};

} // namespace internals
} // namespace everest


#endif // EVEREST_INTERNAL_URL_HPP_