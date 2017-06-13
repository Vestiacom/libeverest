#include "url.hpp"
#include "logger.hpp"

#include <memory>
#include <http_parser.h>

namespace everest {
namespace internals {

URL::URL(const std::string& url)
	: host("0.0.0.0"),
	  port(80)
{
	// Use http-parser's ULR parser
	std::unique_ptr<::http_parser_url> u(new ::http_parser_url);
	::http_parser_url_init(u.get());

	/* Parse a URL; return nonzero on failure */
	if (0 != ::http_parser_parse_url(url.c_str(), url.size(),
	                                 1 /* is_connect == 1, e.g. localhost:1500 */ ,
	                                 u.get())) {
		THROW("Failed to parse connection URI");
	}

	if (u->field_set & (1 << UF_HOST)) {
		host = std::string(&url[u->field_data[::UF_HOST].off],
		                   u->field_data[::UF_HOST].len);
	}

	if (u->field_set & (1 << UF_PORT)) {
		port = u->port;
	}
};

} // namespace internals
} // namespace everest
