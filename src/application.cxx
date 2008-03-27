/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	application.cxx - this file is integral part of `hector' project.

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
#include "application.h"
#include "setup.h"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

HApplication::HApplication( void ) : f_oDOM()
	{
	}

void HApplication::load( char const* const name, char const* const path )
	{
	M_PROLOG
	static char const* const D_INTERFACE_FILE = "interface.xml";
	static char const* const D_TOOLKIT_FILE = "toolkit.xml";
	HStringStream interface( path );
	HStringStream toolkit( path );
	hcore::log( LOG_TYPE::D_INFO ) << "Loading application `" << name << "'." << endl;
	interface << "/" << name << "/" << D_INTERFACE_FILE;
	toolkit << "/" << name << "/" << D_TOOLKIT_FILE;
	f_oDOM.init( interface.raw() );
	f_oDOM.apply_style( toolkit.raw() );
	f_oDOM.parse();
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << interface.raw() << "' as application template." << endl;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << toolkit.raw() << "' as a toolkit library." << endl;
	return;	
	M_EPILOG
	}

void HApplication::run( ORequest& req )
	{
	f_oDOM.save( req.socket()->get_file_descriptor() );
	}

}

