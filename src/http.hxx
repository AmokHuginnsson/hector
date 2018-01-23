/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_HTTP_HXX_INCLUDED
#define HECTOR_HTTP_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>

namespace hector {

namespace HTTP {

extern char const REMOTE_ADDR[];
extern char const SERVER_PORT[];
extern char const HTTPS[];
extern char const HTTP_HOST[];
extern char const HTTP_USER_AGENT[];
extern char const CONTENT_TYPE[];
extern char const CONTENT_TYPE_URLENCODED[];

yaal::hcore::HString decode( yaal::hcore::HString );

}

}

#endif /* #ifndef HECTOR_HTTP_HXX_INCLUDED */

