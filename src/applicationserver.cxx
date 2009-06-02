/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	applicationserver.cxx - this file is integral part of `hector' project.

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

#include <sys/wait.h>

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "applicationserver.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

HApplicationServer::HApplicationServer( void )
	: HServer( setup.f_iMaxConnections ),
	f_oApplications(), f_oPending(), f_oConfiguration(),
	f_oDefaultApplication(), f_oSigChildEvent()
	{
	}

HApplicationServer::~HApplicationServer( void )
	{
	clean_request( 0 );
	}

void HApplicationServer::start( void )
	{
	M_PROLOG
	HSignalService& ss = HSignalServiceFactory::get_instance();
	HSignalService::HHandlerGeneric::ptr_t handler( new HSignalService::HHandlerExternal( this, &HApplicationServer::on_sigchild ) );
	ss.register_handler( SIGCHLD, handler );
	register_file_descriptor_handler( f_oSigChildEvent.get_reader_fd(), &HApplicationServer::process_sigchild );

	static char const* const CONFIGURATION_FILE = "/hector.xml";
	static char const* const NODE_CONFIGURATION = "configuration";
	static char const* const NODE_APPLICATIONS = "applications";
	HStringStream confPath( setup.f_oDataDir );
	confPath << CONFIGURATION_FILE;
	f_oConfiguration.load( HStreamInterface::ptr_t( new HFile( confPath.string() ) ) );
	HXml::HConstNodeProxy hector = f_oConfiguration.get_root();
	for ( HXml::HConstIterator it = hector.begin(); it != hector.end(); ++ it )
		{
		HString const& name = (*it).get_name();
		if ( name == NODE_CONFIGURATION )
			read_configuration( *it );
		else if ( name == NODE_APPLICATIONS )
			read_applications( *it );
		}
	hcore::log( LOG_TYPE::INFO ) << "Statring application server." << endl;
	init_server();
	f_oSocket[ IPC_CHANNEL::CONTROL ]->set_timeout( setup.f_iSocketWriteTimeout );
	f_oSocket[ IPC_CHANNEL::REQUEST ]->set_timeout( setup.f_iSocketWriteTimeout );
	M_EPILOG
	}

void HApplicationServer::read_configuration( HXml::HConstNodeProxy const& configuration )
	{
	M_PROLOG
	static char const* const NODE_DEFAULT_APPLICATION = "default_application";
	static char const* const PROP_NAME = "name";
	for ( HXml::HConstIterator it = configuration.begin(); it != configuration.end(); ++ it )
		{
		HString const& name = (*it).get_name();
		if ( name == NODE_DEFAULT_APPLICATION )
			{
			HXml::HNode::properties_t const& props = (*it).properties();
			HXml::HNode::properties_t::const_iterator nameAttr = props.find( PROP_NAME );
			if ( nameAttr != props.end() )
				f_oDefaultApplication = nameAttr->second;
			}
		}
	M_EPILOG
	}

void HApplicationServer::read_applications( HXml::HConstNodeProxy const& applications )
	{
	M_PROLOG
	static char const* const APP_NODE_NAME = "application";
	static char const* const APP_PROP_NAME_SYMBOL = "symbol";
	static char const* const APP_PROP_NAME_LOAD = "load";
	for ( HXml::HConstIterator it = applications.begin(); it != applications.end(); ++ it )
		{
		HXml::HConstNodeProxy application = *it;
		M_ENSURE( application.get_name() == APP_NODE_NAME );
		HXml::HNode::properties_t const& props = application.properties();
		HXml::HNode::properties_t::const_iterator load = props.find( APP_PROP_NAME_LOAD );
		if ( ( load != props.end() ) && ( lexical_cast<bool>( load->second ) ) )
			{
			HXml::HNode::properties_t::const_iterator symbol = props.find( APP_PROP_NAME_SYMBOL );
			M_ENSURE( ( symbol != props.end() ) && ! symbol->second.is_empty() );
			try
				{
				f_oApplications[ symbol->second ] = HActiveX::get_instance( symbol->second, setup.f_oDataDir );
				}
			catch ( HException& e )
				{
				out << "Failed to load `" << symbol->second << "': " << e.what() << "." << endl;
				hcore::log( LOG_TYPE::WARNING ) << "Failed to load `" << symbol->second << "': " << e.what() << "." << endl;
				}
			}
		}
	M_EPILOG
	}

void HApplicationServer::run( void )
	{
	M_PROLOG
	HProcess::run();
	M_EPILOG
	}

void HApplicationServer::do_service_request( ORequest& a_roRequest )
	{
	M_PROLOG
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
				try
					{
					a_roRequest.decompress_jar( application );
					}
				catch ( HBase64Exception& e )
					{
					hcore::log << e.what() << endl;
					}
				catch ( ORequestException& e )
					{
					hcore::log << e.what() << endl;
					}
				it->second.handle_logic( a_roRequest );
				ORequest::dictionary_ptr_t jar = a_roRequest.compress_jar( application );
				for ( ORequest::dictionary_t::iterator cookieIt = jar->begin(); cookieIt != jar->end(); ++ cookieIt )
					*sock << "Set-Cookie: " << cookieIt->first << "=" << cookieIt->second << ";" << endl;
				*sock << "Content-type: text/html; charset=ISO-8859-2\n" << endl;
				it->second.generate_page( a_roRequest );
				}
			else
				msg << "\n\nno such application: " << application << endl;
			}
		*sock << msg.consume();
		_exit( 0 );
		}
	else
		f_oPending.insert( hcore::make_pair( pid, sock ) );
	M_EPILOG
	}

int HApplicationServer::on_sigchild( int a_iSigNo )
	{
	M_PROLOG
	f_oSigChildEvent.write( &a_iSigNo, sizeof( a_iSigNo ) );
	return ( 1 );
	M_EPILOG
	}

int HApplicationServer::process_sigchild( int )
	{
	M_PROLOG
	int dummy = 0;
	f_oSigChildEvent.read( &dummy, sizeof( dummy ) );
	M_ASSERT( dummy == SIGCHLD );
	clean_request( WNOHANG );
	return ( 0 );
	M_EPILOG
	}

void HApplicationServer::clean_request( int opts )
	{
	M_PROLOG
	int pid = 0;
	int status = 0;
	while ( ! f_oPending.is_empty() && ( ( pid = waitpid( WAIT_ANY, &status, opts | WUNTRACED ) ) > 0 ) )
		{
		pending_t::iterator it = f_oPending.find( pid );
		M_ENSURE( it != f_oPending.end() );
		out << "activex finished with: " << status << "\n";
		if ( WIFSIGNALED( status ) )
			cout << "\tby signal: " << WTERMSIG( status ) << endl;
		else
			cout << "\tnormally: " << WEXITSTATUS( status ) << endl;
		disconnect_client( IPC_CHANNEL::REQUEST, it->second, _( "request serviced" ) );
		f_oPending.erase( it );
		}
	return;
	M_EPILOG
	}

void HApplicationServer::do_restart( HSocket::ptr_t& sock, HString const& appName )
	{
	M_PROLOG
	applications_t::iterator it = f_oApplications.find( appName );
	if ( it != f_oApplications.end() )
		{
		try
			{
			HActiveX& newX = f_oApplications[ appName ] = HActiveX::get_instance( appName, setup.f_oDataDir );
			newX.reload_binary();
			}
		catch ( HException& e )
			{
			hcore::log( LOG_TYPE::WARNING ) << "Failed to load `" << appName << "': " << e.what() << "." << endl;
			*sock << "Failed to load `" << appName << "': " << e.what() << "." << endl;
			}
		}
	else
		{
		*sock << "no such application: " << appName << endl;
		}
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	M_EPILOG
	}

void HApplicationServer::do_status( HSocket::ptr_t& sock )
	{
	M_PROLOG
	*sock << "apps: " << f_oApplications.size() << endl;
	*sock << "new: " << f_oRequests.size() << endl;
	*sock << "pending: " << f_oPending.size() << endl;
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	M_EPILOG
	}

}

