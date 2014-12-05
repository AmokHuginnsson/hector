/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	session.cxx - this file is integral part of `hector' project.

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

#include <yaal/tools/hash.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "session.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector {

HSession::HSession( HString const& remoteAddr_ , HString const& httpUserAgent_ )
	: _id( tools::hash::sha1( remoteAddr_ + httpUserAgent_ + HTime( HTime::TZ::UTC ).string() + randomizer_helper::make_randomizer()() ) ),
	_user(), _groups(), _remoteAddr( remoteAddr_ ), _httpUserAgent( httpUserAgent_ ), _lastSeen( HTime::TZ::UTC ), _persistance()
	{}

yaal::hcore::HString const& HSession::get_id( void ) const
	{ return ( _id ); }

yaal::hcore::HString const& HSession::get_user( void ) const
	{ return ( _user ); }

yaal::hcore::HString const& HSession::get_remote_addr( void ) const
	{ return ( _remoteAddr ); }

yaal::hcore::HString const& HSession::get_http_user_agent( void ) const
	{ return ( _httpUserAgent ); }

HSession::groups_t const& HSession::get_groups( void ) const
	{ return ( _groups ); }

void HSession::set_user( yaal::hcore::HString const& user_ ) {
	_user = user_;
	return;
}

void HSession::add_group( yaal::hcore::HString const& group_ ) {
	M_PROLOG
	_groups.insert( group_ );
	return;
	M_EPILOG
}

}

