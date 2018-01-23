/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_SESSION_HXX_INCLUDED
#define HECTOR_SESSION_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/htime.hxx>
#include <yaal/hcore/hset.hxx>
#include <yaal/hcore/hmap.hxx>

namespace hector {

class HSession {
private:
	typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> dictionary_t;
	typedef yaal::hcore::HSet<yaal::hcore::HString> groups_t;
	yaal::hcore::HString _id;
	yaal::hcore::HString _user;
	groups_t _groups;
	yaal::hcore::HString _remoteAddr;
	yaal::hcore::HString _httpUserAgent;
	yaal::hcore::HTime _lastSeen;
	dictionary_t _persistance;
public:
	HSession( yaal::hcore::HString const&, yaal::hcore::HString const& );
	yaal::hcore::HString const& get_id( void ) const;
	yaal::hcore::HString const& get_user( void ) const;
	yaal::hcore::HString const& get_remote_addr( void ) const;
	yaal::hcore::HString const& get_http_user_agent( void ) const;
	groups_t const& get_groups( void ) const;
	void set_user( yaal::hcore::HString const& );
	void add_group( yaal::hcore::HString const& );
};

}

#endif /* #ifndef HECTOR_SESSION_HXX_INCLUDED */

