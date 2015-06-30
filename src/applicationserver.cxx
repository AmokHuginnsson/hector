/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	applicationserver.cxx - this file is integral part of `hector' project.

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

#include <unistd.h>
#include <sys/wait.h>

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/tools/base64.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "applicationserver.hxx"
#include "http.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

HApplicationServer::HApplicationServer( void )
	: HServer( setup._maxConnections ),
	_applications(), _pending(),
	_configuration(), _defaultApplication(), _sigChildEvent(),
	_db( HDataBase::get_connector() ) {
}

HApplicationServer::~HApplicationServer( void ) {
	clean_request( 0 );
}

void HApplicationServer::start( void ) {
	M_PROLOG
	HSignalService& ss = HSignalService::get_instance();
	ss.register_handler( SIGCHLD, call( &HApplicationServer::on_sigchild, this, _1 ) );
	_dispatcher.register_file_descriptor_handler( _sigChildEvent.get_reader_fd(), call( &HApplicationServer::process_sigchild, this, _1 ) );

	static char const* const CONFIGURATION_FILE = "/hector.xml";
	static char const* const NODE_CONFIGURATION = "configuration";
	static char const* const NODE_APPLICATIONS = "applications";
	HStringStream confPath( setup._dataDir );
	confPath << CONFIGURATION_FILE;
	_configuration.load( make_pointer<HFile>( confPath.string(), HFile::OPEN::READING ) );
	HXml::HConstNodeProxy hector = _configuration.get_root();
	for ( HXml::HConstIterator it = hector.begin(); it != hector.end(); ++ it ) {
		HString const& name = (*it).get_name();
		if ( name == NODE_CONFIGURATION )
			read_configuration( *it );
		else if ( name == NODE_APPLICATIONS )
			read_applications( *it );
	}
	hcore::log( LOG_LEVEL::INFO ) << "Statring application server." << endl;
	init_server();
	_socket[ IPC_CHANNEL::CONTROL ]->set_timeout( setup._socketWriteTimeout );
	_socket[ IPC_CHANNEL::REQUEST ]->set_timeout( setup._socketWriteTimeout );
	_db->connect( setup._databaseName, setup._databaseName, setup._databasePassword );
	M_EPILOG
}

void HApplicationServer::read_configuration( HXml::HConstNodeProxy const& configuration ) {
	M_PROLOG
	static char const* const NODE_DEFAULT_APPLICATION = "default_application";
	static char const* const PROP_NAME = "name";
	for ( HXml::HConstIterator it = configuration.begin(); it != configuration.end(); ++ it ) {
		HString const& name = (*it).get_name();
		if ( name == NODE_DEFAULT_APPLICATION ) {
			HXml::HNode::properties_t const& props = (*it).properties();
			HXml::HNode::properties_t::const_iterator nameAttr = props.find( PROP_NAME );
			if ( nameAttr != props.end() )
				_defaultApplication = nameAttr->second;
		}
	}
	M_EPILOG
}

void HApplicationServer::read_applications( HXml::HConstNodeProxy const& applications ) {
	M_PROLOG
	static char const* const APP_NODE_NAME = "application";
	static char const* const APP_PROP_NAME_SYMBOL = "symbol";
	static char const* const APP_PROP_NAME_LOAD = "load";
	for ( HXml::HConstIterator it = applications.begin(); it != applications.end(); ++ it ) {
		HXml::HConstNodeProxy application = *it;
		M_ENSURE( application.get_name() == APP_NODE_NAME );
		HXml::HNode::properties_t const& props = application.properties();
		HXml::HNode::properties_t::const_iterator load = props.find( APP_PROP_NAME_LOAD );
		if ( ( load != props.end() ) && ( lexical_cast<bool>( load->second ) ) ) {
			HXml::HNode::properties_t::const_iterator symbol = props.find( APP_PROP_NAME_SYMBOL );
			M_ENSURE( ( symbol != props.end() ) && ! symbol->second.is_empty() );
			try {
				_applications[ symbol->second ] = HActiveX::get_instance( symbol->second, setup._dataDir, _db );
			} catch ( HException& e ) {
				out << "Failed to load `" << symbol->second << "': " << e.what() << "." << endl;
				hcore::log( LOG_LEVEL::WARNING ) << "Failed to load `" << symbol->second << "': " << e.what() << "." << endl;
			}
		}
	}
	M_EPILOG
}

HApplicationServer::session_t HApplicationServer::handle_session( ORequest& request_, HApplication::sessions_t& sessions_ ) {
	M_PROLOG
	ORequest::value_t sid( request_.lookup( "sid", ORequest::ORIGIN::COOKIE ) );
	ORequest::value_t remoteAddress( request_.lookup( HTTP::REMOTE_ADDR, ORequest::ORIGIN::ENV ) );
	ORequest::value_t httpUserAgent( request_.lookup( HTTP::HTTP_USER_AGENT, ORequest::ORIGIN::ENV ) );
	session_t session;
	if ( remoteAddress && httpUserAgent ) {
		if ( sid ) {
			HApplication::sessions_t::iterator sessionIt( sessions_.find( *sid ) );
			if ( sessionIt != sessions_.end() ) {
				if ( ( *remoteAddress == sessionIt->second.get_remote_addr() ) && ( *httpUserAgent == sessionIt->second.get_http_user_agent() ) ) {
					out << "got valid session ID: " << *sid << endl;
					session = sessionIt->second;
				} else {
					out << "WARNING! forged/spoofed session ID: " << *sid << " ( " << *remoteAddress << " ?= " << sessionIt->second.get_remote_addr() << " ), ( " << *httpUserAgent << " ?= " << sessionIt->second.get_http_user_agent() << " )" << endl;
					sessions_.erase( sessionIt );
				}
			} else {
				clog << "current SIDs: ";
				transform( sessions_.begin(), sessions_.end(), stream_iterator( clog, " " ), select1st<HApplication::sessions_t::value_type>() );
				clog << endl;
				out << "invalid session ID: " << *sid << endl;
			}
		} else
			out << "sid not set" << endl;
		if ( ! session ) {
			HSession newSession( *remoteAddress, *httpUserAgent );
			session = sessions_.insert( make_pair( newSession.get_id(), newSession ) ).first->second;
			request_.update( "sid", newSession.get_id(), ORequest::ORIGIN::COOKIE );
			out << "setting new SID: " << newSession.get_id() << endl;
		}
	} else
		out << "WARNING! missing: " << ( remoteAddress ? "" : HTTP::REMOTE_ADDR ) << " " << ( httpUserAgent ? "" : HTTP::HTTP_USER_AGENT ) << endl;
	return ( session );
	M_EPILOG
}

void HApplicationServer::do_service_request( ORequest& request_ ) {
	M_PROLOG
	HSocket::ptr_t sock = request_.socket();
	HString application( _defaultApplication );
	if ( request_.lookup( "application", application ) && _defaultApplication.is_empty() ) {
		static HString const err( "\n\nno default application set nor application selected!\n" );
		*sock << err << endl;
		out << err << endl;
	} else {
		applications_t::iterator it = _applications.find( application );
		if ( it != _applications.end() ) {
			out << "using application: " << application << endl;
			try {
				request_.decompress_jar( application );
			} catch ( HBase64Exception& e ) {
				hcore::log << e.what() << endl;
			} catch ( ORequestException& e ) {
				hcore::log << e.what() << endl;
			}
			try {
				session_t session( handle_session( request_, it->second.sessions() ) );
				if ( session )
					it->second.handle_logic( request_, *session );
				int pid = fork();
				if ( ! pid ) {
					try {
						ORequest::dictionary_ptr_t jar = request_.compress_jar( application );
						for ( ORequest::dictionary_t::iterator cookieIt = jar->begin(); cookieIt != jar->end(); ++ cookieIt )
							*sock << "Set-Cookie: " << cookieIt->first << "=" << cookieIt->second << ";" << endl;
						*sock << "Content-type: text/html; charset=ISO-8859-2\n" << endl;
						request_.update( "ssl", request_.is_ssl() ? "ssl-on" : "ssl-off", ORequest::ORIGIN::ENV );
						request_.update( "mobile", request_.is_mobile() ? "mobile-on" : "mobile-off", ORequest::ORIGIN::ENV );
						if ( session )
							it->second.generate_page( request_, *session );
					} catch ( HException const& e ) {
						*sock << e.what() << endl;
					} catch ( ... ) {
						/* Graceful shutdown, frist draft. */
					}
					_exit( 0 );
				} else if ( pid > 0 )
					_pending.insert( hcore::make_pair( pid, sock ) );
				else {
					out << "fork failed!" << endl;
					disconnect_client( IPC_CHANNEL::REQUEST, sock, _( "request dropped - fork failed" ) );
				}
			} catch ( ... ) {
				/* Completly failed to fulfill request. */
			}
		} else {
			static HString const err( "no such application: " );
			*sock << "\n\n" << err << application << endl;
			out << err << application << endl;
			disconnect_client( IPC_CHANNEL::REQUEST, sock, _( "error message generated" ) );
		}
	}
	return;
	M_EPILOG
}

int HApplicationServer::on_sigchild( int sigNo_ ) {
	M_PROLOG
	_sigChildEvent.write( &sigNo_, sizeof( sigNo_ ) );
	return ( 1 );
	M_EPILOG
}

void HApplicationServer::process_sigchild( int ) {
	M_PROLOG
	int dummy = 0;
	_sigChildEvent.read( &dummy, sizeof( dummy ) );
	M_ASSERT( dummy == SIGCHLD );
	clean_request( WNOHANG );
	return;
	M_EPILOG
}

namespace {
#pragma GCC diagnostic ignored "-Wold-style-cast"
template<typename T>
inline int FWD_WEXITSTATUS( T val_ ) {
	return ( WEXITSTATUS( val_ ) );
}
template<typename T>
inline bool FWD_WIFSIGNALED( T val_ ) {
	return ( WIFSIGNALED( val_ ) ? true : false );
}
template<typename T>
inline int FWD_WTERMSIG( T val_ ) {
	return ( WTERMSIG( val_ ) );
}
#pragma GCC diagnostic error "-Wold-style-cast"
}

void HApplicationServer::clean_request( int opts ) {
	M_PROLOG
	int pid = 0;
	int status = 0;
	while ( ! _pending.is_empty() && ( ( pid = waitpid( WAIT_ANY, &status, opts | WUNTRACED ) ) > 0 ) ) {
		pending_t::iterator it = _pending.find( pid );
		M_ENSURE( it != _pending.end() );
		out << "activex finished with: " << status << "\n";
		if ( FWD_WIFSIGNALED( status ) )
			clog << "\tby signal: " << FWD_WTERMSIG( status ) << endl;
		else
			clog << "\tnormally: " << FWD_WEXITSTATUS( status ) << endl;
		if ( _requests.find( it->second->get_file_descriptor() ) != _requests.end() )
			disconnect_client( IPC_CHANNEL::REQUEST, it->second, _( "request serviced" ) );
		_pending.erase( it );
	}
	return;
	M_EPILOG
}

void HApplicationServer::do_restart( HSocket::ptr_t& sock, HString const& appName ) {
	M_PROLOG
	applications_t::iterator it = _applications.find( appName );
	if ( it != _applications.end() ) {
		try {
			HActiveX& newX = _applications[ appName ] = HActiveX::get_instance( appName, setup._dataDir, _db );
			newX.reload_binary();
			*sock << "application `" << appName << "' reloaded successfully" << endl;
		} catch ( HException& e ) {
			hcore::log( LOG_LEVEL::WARNING ) << "Failed to load `" << appName << "': " << e.what() << "." << endl;
			*sock << "Failed to load `" << appName << "': " << e.what() << "." << endl;
		}
	} else {
		*sock << "no such application: " << appName << endl;
	}
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	M_EPILOG
}

void HApplicationServer::do_status( HSocket::ptr_t& sock ) {
	M_PROLOG
	*sock << "apps: " << _applications.size() << endl;
	*sock << "new: " << _requests.size() << endl;
	*sock << "pending: " << _pending.size() << endl;
	*sock << "clients[control]: " << _socket[IPC_CHANNEL::CONTROL]->get_client_count() << endl;
	*sock << "clients[request]: " << _socket[IPC_CHANNEL::REQUEST]->get_client_count() << endl;
	*sock << "application statistics:" << endl;
	for ( applications_t::const_iterator it( _applications.begin() ), end( _applications.end() ); it != end; ++ it )
		*sock << "  " << it->first << ": " << it->second.sessions().get_size() << endl;
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	M_EPILOG
}

}

