/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	session.cxx - this file is integral part of `hector' project.

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

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "session.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector
{

HSession::HSession( HString const& remoteAddr_ , HString const& httpUserAgent_ )
	: _id( hash::sha1( remoteAddr_ + httpUserAgent_ + HTime().string() + randomizer_helper::make_randomizer()() ) ),
	_user(), _groups(), _remoteAddr( remoteAddr_ ), _httpUserAgent( httpUserAgent_ ), _lastSeen(), _persistance()
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

void HSession::set_user( yaal::hcore::HString const& user_ )
	{
	_user = user_;
	return;
	}

void HSession::add_group( yaal::hcore::HString const& group_ )
	{
	M_PROLOG
	_groups.insert( group_ );
	return;
	M_EPILOG
	}

}

