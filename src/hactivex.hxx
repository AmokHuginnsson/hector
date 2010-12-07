/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	oactivex.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_OACTIVEX_HXX_INCLUDED
#define HECTOR_OACTIVEX_HXX_INCLUDED

#include "application.hxx"

namespace hector
{

struct OSession;

class HActiveX
	{
	yaal::hcore::HString _binaryPath;
	yaal::tools::HPlugin::ptr_t _activeX;
	HApplication::ptr_t _application;
public:
	HActiveX( yaal::hcore::HString const& path = yaal::hcore::HString() ) : _binaryPath( path ), _activeX(), _application() {}
	static HActiveX get_instance( yaal::hcore::HString const&, yaal::hcore::HString const& );
	void reload_binary( void );
	void handle_logic( ORequest&, OSession& );
	void generate_page( ORequest const&, OSession const& );
	HApplication::sessions_t& sessions( void );
	HApplication::sessions_t const& sessions( void ) const;
	};

}

#endif /* not HECTOR_OACTIVEX_HXX_INCLUDED */

