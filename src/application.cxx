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

#include <iostream>

#include <yaal/yaal.h>
M_VCSID ( "$Id$" )
#include "application.h"
#include "setup.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

HApplication::HApplication( void )
	: f_oDOM(), f_oProcessor(), PROCESSOR( NULL ), f_oName()
	{
	}

HApplication::~HApplication( void )
	{
	out << "Application `" << f_oName << "' unloaded." << endl;
	}

void HApplication::load( char const* const name, char const* const path )
	{
	M_PROLOG
	static char const* const D_INTERFACE_FILE = "interface.xml";
	static char const* const D_TOOLKIT_FILE = "toolkit.xml";
	static char const* const D_PROCESSOR = "processor";
	f_oName = name;
	HStringStream interface( path );
	HStringStream toolkit( path );
	HStringStream processor( path );
	hcore::log( LOG_TYPE::D_INFO ) << "Loading application `" << f_oName << "'." << endl;
	interface << "/" << f_oName << "/" << D_INTERFACE_FILE;
	toolkit << "/" << f_oName << "/" << D_TOOLKIT_FILE;
	processor << "/" << f_oName << "/" << D_PROCESSOR;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << interface.raw() << "' as application template." << endl;
	f_oDOM.init( interface.raw() );
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << toolkit.raw() << "' as a toolkit library." << endl;
	f_oDOM.apply_style( toolkit.raw() );
	f_oDOM.parse();
	do
		{
		try
			{
			f_oProcessor.load( processor.raw() );
			M_ASSERT( f_oProcessor.is_loaded() );
			out << "processor for `" << f_oName << "' loaded" << endl;
			}
		catch ( HPluginException& e )
			{
			out << "cannot load processor for `" << name << "': " << e.what() << endl;
			break;
			}
		try
			{
			f_oProcessor.resolve( "application_processor", PROCESSOR );
			M_ASSERT( PROCESSOR );
			out << "processor for `" << name << "' connected" << endl;
			}
		catch ( HPluginException& e )
			{
			out << "cannot connect processor for `" << name << "'" << endl;
			}
		}
	while ( 0 );
	return;	
	M_EPILOG
	}

void HApplication::run( ORequest& req )
	{
	if ( PROCESSOR )
		PROCESSOR( *this, req );
	f_oDOM.save( req.socket()->get_file_descriptor() );
	}

}

