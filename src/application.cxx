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

#include <dlfcn.h>
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

HApplication::HApplication( void ) : f_pvProcessor( NULL ), f_oDOM(), PROCESSOR( NULL )
	{
	}

HApplication::~HApplication( void )
	{
	if ( f_pvProcessor )
		dlclose( f_pvProcessor );
	}

template<typename tType>
union caster_t
	{
	void * f_pvObjectPointer;
	tType FUNCTION_POINTER;
	};

template<typename tType>
tType dlsym_wrapper( void* a_pvSpace, char const* const a_pcName )
	{
	caster_t<tType> l_xCaster;
	l_xCaster.f_pvObjectPointer = dlsym( a_pvSpace, a_pcName );
	return ( l_xCaster.FUNCTION_POINTER );
	}

void HApplication::load( char const* const name, char const* const path )
	{
	M_PROLOG
	static char const* const D_INTERFACE_FILE = "interface.xml";
	static char const* const D_TOOLKIT_FILE = "toolkit.xml";
	static char const* const D_PROCESSOR = "processor";
	HStringStream interface( path );
	HStringStream toolkit( path );
	HStringStream processor( path );
	hcore::log( LOG_TYPE::D_INFO ) << "Loading application `" << name << "'." << endl;
	interface << "/" << name << "/" << D_INTERFACE_FILE;
	toolkit << "/" << name << "/" << D_TOOLKIT_FILE;
	processor << "/" << name << "/" << D_PROCESSOR;
	f_oDOM.init( interface.raw() );
	f_oDOM.apply_style( toolkit.raw() );
	f_oDOM.parse();
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << interface.raw() << "' as application template." << endl;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << toolkit.raw() << "' as a toolkit library." << endl;
	f_pvProcessor = dlopen( processor.raw(), RTLD_LAZY | RTLD_LOCAL );
	if ( f_pvProcessor )
		out << "processor for `" << name << "' loaded" << endl;
	else
		out << "cannot load processor for `" << name << "': " << dlerror() << endl;
	PROCESSOR = dlsym_wrapper<HApplication::PROCESSOR_t>( f_pvProcessor, "application_processor" );
	if ( PROCESSOR )
		out << "processor for `" << name << "' connected" << endl;
	else
		out << "cannot connect processor for `" << name << "'" << endl;
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

