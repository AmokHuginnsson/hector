/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <cctype>

#include <yaal/hcore/hexception.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "http.hxx"

using namespace yaal;
using namespace yaal::hcore;

namespace hector {

namespace HTTP {

char const REMOTE_ADDR[] = "REMOTE_ADDR";
char const HTTP_USER_AGENT[] = "HTTP_USER_AGENT";
char const SERVER_PORT[] = "SERVER_PORT";
char const HTTP_HOST[] = "HTTP_HOST";
char const HTTPS[] = "HTTPS";
char const CONTENT_TYPE[] = "CONTENT_TYPE";
char const CONTENT_TYPE_URLENCODED[] = "application/x-www-form-urlencoded";

yaal::hcore::HString decode( yaal::hcore::HString data_ ) {
	M_PROLOG
	data_.replace( "+", " " );
	for ( int i( 0 ); i < static_cast<int>( data_.get_length() ); ++ i ) {
		if ( data_[i] == '%' ) {
			int p( i + 1 );
			if ( p == data_.get_length() ) {
				break;
			}
			int nibble( tolower( static_cast<int>( data_[p].get() ) ) );
			if ( ! isxdigit( nibble ) ) {
				continue;
			}
			if ( isdigit( nibble ) ) {
				nibble -= '0';
			} else {
				nibble -= 'a';
				nibble += 10;
			}
			char repl( static_cast<char>( static_cast<u8_t>( nibble ) << 4 ) );
			++ p;
			if ( p == data_.get_length() ) {
				break;
			}
			nibble = tolower( static_cast<int>( data_[p].get() ) );
			if ( ! isxdigit( nibble ) ) {
				continue;
			}
			if ( isdigit( nibble ) ) {
				nibble -= '0';
			} else {
				nibble -= 'a';
				nibble += 10;
			}
			repl = ( repl | static_cast<char>( nibble ) );
			data_.replace( i, 3, &repl, 1 );
		}
	}
	return ( data_ );
	M_EPILOG
}

}

}

