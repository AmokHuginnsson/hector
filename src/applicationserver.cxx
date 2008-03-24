/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	applicationserver.cxx - this file is integral part of `hector' project.

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
#include "applicationserver.h"
#include "setup.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

HApplicationServer::HApplicationServer( void )
	: HServer( setup.f_iMaxConnections ), f_oApplications(), f_oConfiguration()
	{
	}

void HApplicationServer::start( void )
	{
	static char const* const D_SOCK_NAME = "/hector.sock";
	static char const* const D_CONFIGURATION_FILE = "/hector.xml";
	static char const* const D_APP_NODE_NAME = "application";
	static char const* const D_APP_PROP_NAME_SYMBOL = "symbol";
	static char const* const D_APP_PROP_NAME_LOAD = "load";
	HString sockPath( setup.f_oSocketRoot );
	sockPath += D_SOCK_NAME;
	HStringStream confPath( setup.f_oDataDir );
	confPath << D_CONFIGURATION_FILE;
	f_oConfiguration.init( confPath.raw() );
	f_oConfiguration.parse( "/hector/applications/application" );
	HXml::HConstNodeProxy applications = f_oConfiguration.get_root();
	for ( HXml::HConstIterator it = applications.begin(); it != applications.end(); ++ it )
		{
		HXml::HConstNodeProxy application = *it;
		M_ENSURE( application.get_name() == D_APP_NODE_NAME );
		HXml::HNode::properties_t const& props = application.properties();
		HXml::HNode::properties_t::const_iterator load = props.find( D_APP_PROP_NAME_LOAD );
		if ( ( load != props.end() ) && ( to_bool( load->second ) ) )
			{
			HXml::HNode::properties_t::const_iterator symbol = props.find( D_APP_PROP_NAME_SYMBOL );
			M_ENSURE( ( symbol != props.end() ) && ! symbol->second.is_empty() );
			HApplication::ptr_t app( new HApplication() );
			app->load( symbol->second, setup.f_oDataDir );
			f_oApplications.insert( symbol->second, app );
			}
		}
	hcore::log( LOG_TYPE::D_INFO ) << "Statring application server." << endl;
	init_server( sockPath );
	f_oSocket.set_timeout( setup.f_iSocketWriteTimeout );
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << sockPath << "' as IPC inteface." << endl;
	}

void HApplicationServer::run( void )
	{
	HProcess::run();
	}

}

