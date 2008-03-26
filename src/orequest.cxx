/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.cxx - this file is integral part of `hector' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You can not use any part of sources of this software.
  2. You can not redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you can not demand any fees
     for this software.
     You can not even demand cost of the carrier (CD for example).
  5. You can not include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#include <yaal/yaal.h>
M_VCSID ( "$Id$" )
#include "orequest.h"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector
{

ORequest::ORequest( void )
	: f_oEnvironment( new dictionary_t() ), f_oCookies( new dictionary_t() ),
	f_oGET( new dictionary_t() ), f_oPOST( new dictionary_t() )
	{
	}

bool ORequest::lookup( HString const& key, HString& value ) const
	{
	dictionary_t::const_iterator it = f_oEnvironment->find( key );
	bool bFound = false;
	( ! bFound ) && ( bFound = ( ( it = f_oEnvironment->find( key ) ) != f_oEnvironment->end() ) ) && ( value = it->second );
	( ! bFound ) && ( bFound = ( ( it = f_oCookies->find( key ) )     != f_oCookies->end() ) )     && ( value = it->second );
	( ! bFound ) && ( bFound = ( ( it = f_oPOST->find( key ) )        != f_oPOST->end() ) )        && ( value = it->second );
	( ! bFound ) && ( bFound = ( ( it = f_oGET->find( key ) )         != f_oGET->end() ) )         && ( value = it->second );
	return ( ! bFound );
	}

}

