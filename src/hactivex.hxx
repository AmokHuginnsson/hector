/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	oactivex.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_OACTIVEX_HXX_INCLUDED
#define HECTOR_OACTIVEX_HXX_INCLUDED

#include "application.hxx"

namespace hector {

struct HSession;

class HActiveX {
	yaal::hcore::HString _binaryPath;
	yaal::tools::HPlugin::ptr_t _activeX;
	HApplication::ptr_t _application;
public:
	HActiveX( yaal::hcore::HString const& path = yaal::hcore::HString() )
		: _binaryPath( path )
		, _activeX()
		, _application() {
	}
	static HActiveX get_instance(
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		HApplication::MODE,
		yaal::dbwrapper::HDataBase::ptr_t
	);
	void reload_binary( void );
	void handle_logic( ORequest&, HSession& );
	void generate_page( ORequest const&, HSession const& );
	HApplication const& app( void ) const;
	HApplication::sessions_t& sessions( void );
};

typedef yaal::hcore::HExceptionT<HActiveX> HActiveXException;

}

#endif /* not HECTOR_OACTIVEX_HXX_INCLUDED */

