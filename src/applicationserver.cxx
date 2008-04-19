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

#include <sys/wait.h>
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
	: HServer( setup.f_iMaxConnections ),
	f_oApplications(), f_oConfiguration(), f_oDefaultApplication()
	{
	}

HApplicationServer::~HApplicationServer( void )
	{
	}

void HApplicationServer::start( void )
	{
	M_PROLOG
	static char const* const D_SOCK_NAME = "/hector.sock";
	static char const* const D_CONFIGURATION_FILE = "/hector.xml";
	static char const* const D_NODE_CONFIGURATION = "configuration";
	static char const* const D_NODE_APPLICATIONS = "applications";
	HString sockPath( setup.f_oSocketRoot );
	sockPath += D_SOCK_NAME;
	HStringStream confPath( setup.f_oDataDir );
	confPath << D_CONFIGURATION_FILE;
	f_oConfiguration.load( confPath.raw() );
	HXml::HConstNodeProxy hector = f_oConfiguration.get_root();
	for ( HXml::HConstIterator it = hector.begin(); it != hector.end(); ++ it )
		{
		HString const& name = (*it).get_name();
		if ( name == D_NODE_CONFIGURATION )
			read_configuration( *it );
		else if ( name == D_NODE_APPLICATIONS )
			read_applications( *it );
		}
	hcore::log( LOG_TYPE::D_INFO ) << "Statring application server." << endl;
	init_server( sockPath );
	f_oSocket.set_timeout( setup.f_iSocketWriteTimeout );
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << sockPath << "' as IPC inteface." << endl;
	M_EPILOG
	}

void HApplicationServer::read_configuration( HXml::HConstNodeProxy const& configuration )
	{
	M_PROLOG
	static char const* const D_NODE_DEFAULT_APPLICATION = "default_application";
	static char const* const D_PROP_NAME = "name";
	for ( HXml::HConstIterator it = configuration.begin(); it != configuration.end(); ++ it )
		{
		HString const& name = (*it).get_name();
		if ( name == D_NODE_DEFAULT_APPLICATION )
			{
			HXml::HNode::properties_t const& props = (*it).properties();
			HXml::HNode::properties_t::const_iterator nameAttr = props.find( D_PROP_NAME );
			if ( nameAttr != props.end() )
				f_oDefaultApplication = nameAttr->second;
			}
		}
	M_EPILOG
	}

void HApplicationServer::read_applications( HXml::HConstNodeProxy const& applications )
	{
	M_PROLOG
	static char const* const D_APP_NODE_NAME = "application";
	static char const* const D_APP_PROP_NAME_SYMBOL = "symbol";
	static char const* const D_APP_PROP_NAME_LOAD = "load";
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
	M_EPILOG
	}

void HApplicationServer::run( void )
	{
	HProcess::run();
	}

void HApplicationServer::do_service_request( ORequest& a_roRequest )
	{
	int pid = fork();
	HSocket::ptr_t sock = a_roRequest.socket();
	if ( ! pid )
		{
		HStringStream msg;
		HString application( f_oDefaultApplication );
		if ( a_roRequest.lookup( "application", application ) && f_oDefaultApplication.is_empty() )
			msg = "no default application set nor application selected!\n";
		else
			{
			applications_t::iterator it = f_oApplications.find( application );
			if ( it != f_oApplications.end() )
				{
				out << "using application: " << application << endl;
				it->second->run( a_roRequest );
				}
			else
				msg << "no such application: " << application << endl;
			}
		*sock << msg.consume();
		_exit( 0 );
		}
	else
		{
		waitpid( pid, NULL, 0 );
		disconnect_client( sock, _( "request serviced" ) );
		}
	}

}

