/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <unistd.h>
#include <sys/wait.h>

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/tools/base64.hxx>
#include <yaal/dbwrapper/dbwrapper.hxx>
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
	: HServer( setup._maxConnections )
	, _applications()
	, _pending()
	, _configuration()
	, _defaultApplication()
	, _sigChildEvent()
	, _dbConnections() {
}

HApplicationServer::~HApplicationServer( void ) {
	clean_request( 0 );
}

yaal::dbwrapper::HDataBase::ptr_t HApplicationServer::get_db_connection( yaal::hcore::HString const& dsn_ ) {
	M_PROLOG
	db_connections_t::iterator it( _dbConnections.find( dsn_ ) );
	HDataBase::ptr_t dbConnection;
	if ( it != _dbConnections.end() ) {
		dbConnection = it->second;
	} else {
		dbConnection = dbwrapper::util::connect( dsn_ );
		_dbConnections.insert( make_pair( dsn_, dbConnection ) );
	}
	return ( dbConnection );
	M_EPILOG
}

void HApplicationServer::start( void ) {
	M_PROLOG
	HSignalService& ss = HSignalService::get_instance();
	ss.register_handler( SIGCHLD, call( &HApplicationServer::on_sigchild, this, _1 ) );
	_dispatcher.register_file_descriptor_handler( _sigChildEvent.out(), call( &HApplicationServer::process_sigchild, this, _1 ) );

	static char const* const CONFIGURATION_FILE = "/hector.xml";
	static char const* const NODE_CONFIGURATION = "configuration";
	static char const* const NODE_APPLICATIONS = "applications";
	HStringStream confPath( setup._dataDir );
	confPath << CONFIGURATION_FILE;
	_configuration.load( make_pointer<HFile>( confPath.string(), HFile::OPEN::READING ) );
	HXml::HConstNodeProxy hector = _configuration.get_root();
	for ( HXml::HConstIterator it = hector.begin(); it != hector.end(); ++ it ) {
		HString const& name = (*it).get_name();
		if ( name == NODE_CONFIGURATION ) {
			read_configuration( *it );
		} else if ( name == NODE_APPLICATIONS ) {
			read_applications( *it );
		}
	}
	hcore::log( LOG_LEVEL::INFO ) << "Starting application server." << endl;
	init_server();
	_socket[ IPC_CHANNEL::CONTROL ]->set_timeout( setup._socketWriteTimeout );
	_socket[ IPC_CHANNEL::REQUEST ]->set_timeout( setup._socketWriteTimeout );
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
			if ( nameAttr != props.end() ) {
				_defaultApplication = nameAttr->second;
			}
		}
	}
	M_EPILOG
}

void HApplicationServer::read_applications( HXml::HConstNodeProxy const& applications ) {
	M_PROLOG
	static char const APP_NODE_NAME[] = "application";
	static char const APP_PROP_NAME_ID[] = "id";
	static char const APP_PROP_NAME_LOAD[] = "load";
	static char const APP_PROP_NAME_CODE[] = "code";
	static char const APP_PROP_NAME_NAME[] = "name";
	static char const APP_PROP_NAME_MODE[] = "mode";
	static HApplication::MODE DEFAULT_MODE = HApplication::MODE::GET;
	for ( HXml::HConstNodeProxy application : applications ) {
		if ( application.get_type() != HXml::HNode::TYPE::NODE ) {
			continue;
		}
		M_ENSURE( application.get_name() == APP_NODE_NAME );
		HXml::HNode::properties_t const& props = application.properties();
		HXml::HNode::properties_t::const_iterator load = props.find( APP_PROP_NAME_LOAD );
		if ( ( load != props.end() ) && ( lexical_cast<bool>( load->second ) ) ) {
			xml::value_t id( xml::try_attr_val( application, APP_PROP_NAME_ID ) );
			xml::value_t codeVal( xml::try_attr_val( application, APP_PROP_NAME_CODE ) );
			xml::value_t nameVal( xml::try_attr_val( application, APP_PROP_NAME_NAME ) );
			xml::value_t modeVal( xml::try_attr_val( application, APP_PROP_NAME_MODE ) );
			M_ENSURE( !! id && ! id->is_empty() );
			try {
				HApplication::MODE mode( DEFAULT_MODE );
				if ( !! modeVal ) {
					HString modeStr( *modeVal );
					modeStr.lower();
					if ( modeStr == "get" ) {
						mode = HApplication::MODE::GET;
					} else if ( modeStr == "post" ) {
						mode = HApplication::MODE::POST;
					} else {
						throw HApplicationServerException( "bad application mode: "_ys.append( *modeVal ) );
					}
				}
				_applications.insert(
					make_pair(
						*id,
						HActiveX::get_instance(
							this,
							*id,
							!! codeVal ? *codeVal : *id,
							!! nameVal ? *nameVal : *id,
							setup._dataDir,
							mode
						)
					)
				);
			} catch ( HException& e ) {
				OUT << "Failed to load `" << *id << "': " << e.what() << "." << endl;
				hcore::log( LOG_LEVEL::WARNING ) << "Failed to load `" << *id << "': " << e.what() << "." << endl;
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
					OUT << "got valid session ID: " << *sid << endl;
					session = sessionIt->second;
				} else {
					OUT << "WARNING! forged/spoofed session ID: " << *sid << " ( " << *remoteAddress << " ?= " << sessionIt->second.get_remote_addr() << " ), ( " << *httpUserAgent << " ?= " << sessionIt->second.get_http_user_agent() << " )" << endl;
					sessions_.erase( sessionIt );
				}
			} else {
				clog << "current SIDs: ";
				transform( sessions_.begin(), sessions_.end(), stream_iterator( clog, " " ), select1st<HApplication::sessions_t::value_type>() );
				clog << endl;
				OUT << "invalid session ID: " << *sid << endl;
			}
		} else {
			OUT << "sid not set" << endl;
		}
		if ( ! session ) {
			HSession newSession( *remoteAddress, *httpUserAgent );
			session = sessions_.insert( make_pair( newSession.get_id(), newSession ) ).first->second;
			request_.update( "sid", newSession.get_id(), ORequest::ORIGIN::COOKIE );
			OUT << "setting new SID: " << newSession.get_id() << endl;
		}
	} else {
		OUT << "WARNING! missing: " << ( remoteAddress ? "" : HTTP::REMOTE_ADDR ) << " " << ( httpUserAgent ? "" : HTTP::HTTP_USER_AGENT ) << endl;
	}
	return ( session );
	M_EPILOG
}

void HApplicationServer::do_service_request( ORequest& request_ ) {
	M_PROLOG
	HStreamInterface::ptr_t sock = request_.socket();
	HString application( _defaultApplication );
	if ( request_.lookup( "application", application ) && _defaultApplication.is_empty() ) {
		static HString const err( "\n\nno default application set nor application selected!\n" );
		*sock << err << endl;
		OUT << err << endl;
	} else {
		applications_t::iterator it = _applications.find( application );
		if ( it != _applications.end() ) {
			OUT << "using application: " << application << endl;
			try {
				request_.decompress_jar( application );
			} catch ( HBase64Exception& e ) {
				hcore::log << e.what() << endl;
			} catch ( ORequestException& e ) {
				hcore::log << e.what() << endl;
			}
			try {
				session_t session( handle_session( request_, it->second.sessions() ) );
				if ( !! session ) {
					it->second.handle_logic( request_, *session );
				}
				int pid = fork();
				if ( ! pid ) {
					try {
						ORequest::dictionary_ptr_t jar = request_.compress_jar( application );
						for ( ORequest::dictionary_t::iterator cookieIt = jar->begin(); cookieIt != jar->end(); ++ cookieIt ) {
							*sock << "Set-Cookie: " << cookieIt->first << "=" << cookieIt->second << ";" << endl;
						}
						*sock << "Content-type: text/html; charset=UTF-8\n" << endl;
						request_.update( "ssl", request_.is_ssl() ? "ssl-on" : "ssl-off", ORequest::ORIGIN::ENV );
						request_.update( "mobile", request_.is_mobile() ? "mobile-on" : "mobile-off", ORequest::ORIGIN::ENV );
						if ( !! session ) {
							it->second.generate_page( request_, *session );
						}
					} catch ( HException const& e ) {
						*sock << e.what() << endl;
					} catch ( ... ) {
						/* Graceful shutdown, frist draft. */
					}
					_exit( 0 );
				} else if ( pid > 0 ) {
					_pending.insert( hcore::make_pair( pid, sock ) );
				} else {
					OUT << "fork failed!" << endl;
					disconnect_client( IPC_CHANNEL::REQUEST, sock, _( "request dropped - fork failed" ) );
				}
			} catch ( ... ) {
				/* Completly failed to fulfill request. */
			}
		} else {
			static HString const err( "no such application: " );
			*sock << "\n\n" << err << application << endl;
			OUT << err << application << endl;
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

void HApplicationServer::process_sigchild( HIODispatcher::stream_t& ) {
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
		OUT << "activex finished with: " << status << "\n";
		if ( FWD_WIFSIGNALED( status ) )
			clog << "\tby signal: " << FWD_WTERMSIG( status ) << endl;
		else
			clog << "\tnormally: " << FWD_WEXITSTATUS( status ) << endl;
		if ( _requests.find( it->second.raw() ) != _requests.end() ) {
			disconnect_client( IPC_CHANNEL::REQUEST, it->second, _( "request serviced" ) );
		}
		_pending.erase( it );
	}
	return;
	M_EPILOG
}

void HApplicationServer::do_restart( HStreamInterface::ptr_t sock, HString const& appName ) {
	M_PROLOG
	applications_t::iterator it = _applications.find( appName );
	if ( it != _applications.end() ) {
		try {
			HActiveX& newX = _applications[ appName ] = HActiveX::get_instance(
				this,
				it->second.app().id(),
				it->second.app().code(),
				it->second.app().name(),
				setup._dataDir,
				it->second.app().get_mode()
			);
			newX.reload_binary();
			*sock << "application `" << appName << "' restarted successfully" << endl;
		} catch ( HException& e ) {
			hcore::log( LOG_LEVEL::WARNING ) << "Failed to load `" << appName << "': " << e.what() << "." << endl;
			*sock << "Failed to load `" << appName << "': " << e.what() << "." << endl;
		}
	} else {
		*sock << "no such application: " << appName << endl;
	}
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	return;
	M_EPILOG
}

void HApplicationServer::do_reload( HStreamInterface::ptr_t sock, HString const& appName ) {
	M_PROLOG
	applications_t::iterator it = _applications.find( appName );
	if ( it != _applications.end() ) {
	} else {
		*sock << "no such application: " << appName << endl;
	}
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	return;
	M_EPILOG
}

void HApplicationServer::do_status( HStreamInterface::ptr_t& sock ) {
	M_PROLOG
	*sock << "apps: " << _applications.size() << endl;
	*sock << "new: " << _requests.size() << endl;
	*sock << "pending: " << _pending.size() << endl;
	*sock << "application statistics:" << endl;
	for ( applications_t::const_iterator it( _applications.begin() ), end( _applications.end() ); it != end; ++ it ) {
		*sock << "  " << it->first << ": " << it->second.app().sessions().get_size() << endl;
	}
	disconnect_client( IPC_CHANNEL::CONTROL, sock, _( "request serviced" ) );
	return;
	M_EPILOG
}

}

