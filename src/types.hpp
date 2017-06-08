#ifndef EVEREST_TYPES_HPP_
#define EVEREST_TYPES_HPP_

#include <http_parser.h>
#include <list>

namespace everest {

typedef ::http_method HTTPMethod;

typedef std::list<std::pair<std::string, std::string>> headers_t;

} // namespace everest

#endif // EVEREST_TYPES_HPP_