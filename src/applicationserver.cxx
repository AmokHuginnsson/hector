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
	: HServer( setup._maxConnections ),
	_applications(), _pending(),
	_configuration(), _defaultApplication(), _sigChildEvent()
	{
	}

HApplicationServer::~HApplicationServer( void )
	{
	clean_request( 0 );
	}

void HApplicationServer::start( void )
	{
	M_PROLOG
	HSignalService& ss = HSignalService::get_instance();
	ss.register_handler( SIGCHLD, call( &HApplicationServer::on_sigchild, this, _1 ) );
	_dispatcher.register_file_descriptor_handler( _sigChildEvent.get_reader_fd(), call( &HApplicationServer::process_sigchild, this, _1 ) );

	static char const* const CONFIGURATION_FILE = "/hector.xml";
	static char const* const NODE_CONFIGURATION = "configuration";
	static char const* const NODE_APPLICATIONS = "applications";
	HStringStream confPath( setup._dataDir );
	confPath << CONFIGURATION_FILE;
	_configuration.load( HStreamInterface::ptr_t( new HFile( confPath.string(), HFile::OPEN::READING ) ) );
	HXml::HConstNodeProxy hector = _configuration.get_root();
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
	_socket[ IPC_CHANNEL::CONTROL ]->set_timeout( setup._socketWriteTimeout );
	_socket[ IPC_CHANNEL::REQUEST ]->set_timeout( setup._socketWriteTimeout );
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
				_defaultApplication = nameAttr->second;
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
				_applications[ symbol->second ] = HActiveX::get_instance( symbol->second, setup._dataDir );
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

HApplicationServer::session_t HApplicationServer::handle_session( ORequest& request_, HApplication::sessions_t& sessions_ )
	{
	M_PROLOG
	static char const REMOTE_ADDR[] = "REMOTE_ADDR";
	static char const HTTP_USER_AGENT[] = "HTTP_USER_AGENT";
	ORequest::value_t sid( request_.lookup( "sid", ORequest::ORIGIN::COOKIE ) );
	ORequest::value_t remoteAddress( request_.lookup( REMOTE_ADDR, ORequest::ORIGIN::ENV ) );
	ORequest::value_t httpUserAgent( request_.lookup( HTTP_USER_AGENT, ORequest::ORIGIN::ENV ) );
	session_t session;
	if ( remoteAddress && httpUserAgent )
		{
		if ( sid )
			{
			HApplication::sessions_t::iterator sessionIt( sessions_.find( *sid ) );
			if ( sessionIt != sessions_.end() )
				{
				if ( ( *remoteAddress == sessionIt->second._remoteAddr ) && ( *httpUserAgent == sessionIt->second._httpUserAgent ) )
					{
					out << "got valid session ID: " << *sid << endl;
					session = sessionIt->second;
					}
				else
					{
					out << "WARNING! forged/spoofed session ID: " << *sid << "( " << *remoteAddress << " ?= " << sessionIt->second._remoteAddr << " ), ( " << *httpUserAgent << " ?= " << sessionIt->second._httpUserAgent << " )" << endl;
					sessions_.erase( sessionIt );
					}
				}
			else
				{
				clog << "current SIDs: ";
				transform( sessions_.begin(), sessions_.end(), stream_iterator( clog, " " ), select1st<HApplication::sessions_t::value_type>() );
				clog << endl;
				out << "invalid session ID: " << *sid << endl;
				}
			}
		else
			out << "sid not set" << endl;
		if ( ! session )
			{
			OSession newSession;
			newSession._remoteAddr = *remoteAddress;
			newSession._httpUserAgent = *httpUserAgent;
			HString newSid( hash::sha1( newSession._remoteAddr + newSession._httpUserAgent + HTime().string() + randomizer_helper::make_randomizer()() ) );
			session = sessions_.insert( make_pair( newSid, newSession ) ).first->second;
			request_.update( "sid", newSid, ORequest::ORIGIN::COOKIE );
			out << "setting new SID: " << newSid << endl;
			}
		}
	else
		out << "WARNING! missing: " << ( remoteAddress ? "" : REMOTE_ADDR ) << " " << ( httpUserAgent ? "" : HTTP_USER_AGENT ) << endl;
	return ( session );
	M_EPILOG
	}

void HApplicationServer::do_service_request( ORequest& request_ )
	{
	M_PROLOG
	HSocket::ptr_t sock = request_.socket();
	HStringStream msg;
	HString application( _defaultApplication );
	if ( request_.lookup( "application", application ) && _defaultApplication.is_empty() )
		msg = "no default application set nor application selected!\n";
	else
		{
		applications_t::iterator it = _applications.find( application );
		if ( it != _applications.end() )
			{
			out << "using application: " << application << endl;
			try
				{
				request_.decompress_jar( application );
				}
			catch ( HBase64Exception& e )
				{
				hcore::log << e.what() << endl;
				}
			catch ( ORequestException& e )
				{
				hcore::log << e.what() << endl;
				}
			try
				{
				session_t session( handle_session( request_, it->second.sessions() ) );
				if ( session )
					it->second.handle_logic( request_, *session );
				int pid = fork();
				if ( ! pid )
					{
					try
						{
						ORequest::dictionary_ptr_t jar = request_.compress_jar( application );
						for ( ORequest::dictionary_t::iterator cookieIt = jar->begin(); cookieIt != jar->end(); ++ cookieIt )
							*sock << "Set-Cookie: " << cookieIt->first << "=" << cookieIt->second << ";" << endl;
						*sock << "Content-type: text/html; charset=ISO-8859-2\n" << endl;
						if ( session )
							it->second.generate_page( request_, *session );
						*sock << msg.consume();
						}
					catch ( ... )
						{
						/* Graceful shutdown, frist draft. */
						}
					_exit( 0 );
					}
				else if ( pid > 0 )
					_pending.insert( hcore::make_pair( pid, sock ) );
				else
					out << "fork failed!" << endl;
				}
			catch ( ... )
				{
				/* Completly failed to fulfill request. */
				}
			}
		else
			msg << "\n\nno such application: " << application << endl;
		}
	return;
	M_EPILOG
	}

int HApplicationServer::on_sigchild( int sigNo_ )
	{
	M_PROLOG
	_sigChildEvent.write( &sigNo_, sizeof( sigNo_ ) );
	return ( 1 );
	M_EPILOG
	}

void HApplicationServer::process_sigchild( int )
	{
	M_PROLOG
	int dummy = 0;
	_sigChildEvent.read( &dummy, sizeof( dummy ) );
	M_ASSERT( dummy == SIGCHLD );
	clean_request( WNOHANG );
	return;
	M_EPILOG
	}

void HApplicationServer::clean_request( int opts )
	{
	M_PROLOG
	int pid = 0;
	int status = 0;
	while ( ! _pending.is_empty() && ( ( pid = waitpid( WAIT_ANY, &status, opts | WUNTRACED ) ) > 0 ) )
		{
		pending_t::iterator it = _pending.find( pid );
		M_ENSURE( it != _pending.end() );
		out << "activex finished with: " << status << "\n";
		if ( WIFSIGNALED( status ) )
			clog << "\tby signal: " << WTERMSIG( status ) << endl;
		else
			clog << "\tnormally: " << WEXITSTATUS( status ) << endl;
		if ( _requests.find( it->second->get_file_descriptor() ) != _requests.end() )
			disconnect_client( IPC_CHANNEL::REQUEST, it->second, _( "request serviced" ) );
		_pending.erase( it );
		}
	return;
	M_EPILOG
	}

void HApplicationServer::do_restart( HSocket::ptr_t& sock, HString const& appName )
	{
	M_PROLOG
	applications_t::iterator it = _applications.find( appName );
	if ( it != _applications.end() )
		{
		try
			{
			HActiveX& newX = _applications[ appName ] = HActiveX::get_instance( appName, setup._dataDir );
			newX.reload_binary();
			*sock << "application `" << appName << "' reloaded successfully" << endl;
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
	*sock << "apps: " << _applications.size() << endl;
	*sock << "new: " << _requests.size() << endl;
	*sock << "pending: " << _pending.size() << endl;
	*sock << "application statistics:" << endl;
	for ( applications_t::const_iterator it( _applications.begin() ), end( _applications.end() ); it != end; ++ it )
		*sock << "  " << it->first << ": " << it->second.sessions().get_size() << endl;
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	M_EPILOG
	}

}

