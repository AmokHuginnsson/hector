/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	http.cxx - this file is integral part of `hector' project.

  i.  You may not make any changes in Copyright information.
  ii. You must attach Copyright information to any part of every copy
      of this software.

Copyright:

 You can use this software free of charge and you can redistribute its binary
 package freely but:
  1. You are not allowed to use any part of sources of this software.
  2. You are not allowed to redistribute any part of sources of this software.
  3. You are not allowed to reverse engineer this software.
  4. If you want to distribute a binary package of this software you cannot
     demand any fees for it. You cannot even demand
     a return of cost of the media or distribution (CD for example).
  5. You cannot involve this software in any commercial activity (for example
     as a free add-on to paid software or newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

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

