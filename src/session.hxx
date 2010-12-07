/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	session.hxx - this file is integral part of `hector' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You cannot use any part of sources of this software.
  2. You cannot redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you cannot demand any fees
     for this software.
     You cannot even demand cost of the carrier (CD for example).
  5. You cannot include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#ifndef HECTOR_SESSION_HXX_INCLUDED
#define HECTOR_SESSION_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/htime.hxx>

namespace hector
{

struct OSession
	{
	yaal::hcore::HString _user;
	typedef yaal::hcore::HSet<yaal::hcore::HString> groups_t;
	groups_t _groups;
	yaal::hcore::HString _remoteAddr;
	yaal::hcore::HString _httpUserAgent;
	yaal::hcore::HTime _lastSeen;
	OSession( void ) : _user(), _groups(), _remoteAddr(), _httpUserAgent(), _lastSeen() {}
	};

}

#endif /* #ifndef HECTOR_SESSION_HXX_INCLUDED */

