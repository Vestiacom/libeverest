#include "encoding.hpp"

namespace {

const char HEX2DEC[55] = {
	0, 1, 2, 3,  4, 5, 6, 7,  8, 9, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 10, 11, 12, 13, 14, 15
};

} // namespace

namespace everest {
namespace internals {

#define __x2d__(s) HEX2DEC[*(s)-48]
#define __x2d2__(s) (__x2d__(s) << 4 | __x2d__(s+1))

// From https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
std::wstring decode(const char* s)
{
	unsigned char b;
	std::wstring ws;
	while (*s) {
		if (*s == '%')
			if ((b = __x2d2__(s + 1)) >= 0x80) {
				if (b >= 0xE0) { // three byte codepoint
					ws += ((b & 0b00001111) << 12) | ((__x2d2__(s + 4) & 0b00111111) << 6) | (__x2d2__(s + 7) & 0b00111111);
					s += 9;
				} else { // two byte codepoint
					ws += (__x2d2__(s + 4) & 0b00111111) | (b & 0b00000011) << 6;
					s += 6;
				}
			} else { // one byte codepoints
				ws += b;
				s += 3;
			}
		else { // no %
			ws += *s;
			s++;
		}
	}
	return ws;
}

} // namespace internals
} // namespace everest