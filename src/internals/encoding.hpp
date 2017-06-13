#ifndef EVEREST_INTERNALS_ENCODING_HPP_
#define EVEREST_INTERNALS_ENCODING_HPP_

#include <sstream>
#include <iostream>
#include "../types.hpp"

namespace everest {
namespace internals {


// TODO: Replace with "in-place" encoding (realloc)
// std::string encode(const std::string& sSrc);

// TODO: Replace with "in-place" decoding
std::wstring decode(const char* s);


} // namespace internals
} // everest

#endif // EVEREST_INTERNALS_ENCODING_HPP_
