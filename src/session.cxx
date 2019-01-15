/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <yaal/tools/hash.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "session.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector {

HSession::HSession( HString const& remoteAddr_ , HString const& httpUserAgent_ )
	: _id(
		tools::hash::to_string(
			tools::hash::sha1(
				remoteAddr_ + httpUserAgent_ + HTime( HTime::TZ::UTC ).string() + random::HRandomNumberGenerator()()
			)
		)
	)
	, _user()
	, _groups()
	, _remoteAddr( remoteAddr_ )
	, _httpUserAgent( httpUserAgent_ )
	, _lastSeen( HTime::TZ::UTC )
	, _persistance() {
}

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

