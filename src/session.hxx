/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	session.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_SESSION_HXX_INCLUDED
#define HECTOR_SESSION_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/htime.hxx>

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

