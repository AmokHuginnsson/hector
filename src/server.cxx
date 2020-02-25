/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <unistd.h>
#include <sys/stat.h>

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/ansi.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "server.hxx"
#include "http.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::ansi;
using namespace yaal::tools;

namespace hector {

char const* const HServer::CONTROL_PROTO::SHUTDOWN = "shutdown";
char const* const HServer::CONTROL_PROTO::RELOAD = "reload";
char const* const HServer::CONTROL_PROTO::RESTART = "restart";
char const* const HServer::CONTROL_PROTO::STATUS = "status";
char const* const HServer::REQUEST_PROTO::ENV = "env";
char const* const HServer::REQUEST_PROTO::COOKIE = "cookie";
char const* const HServer::REQUEST_PROTO::GET = "get";
char const* const HServer::REQUEST_PROTO::POST = "post";
char const* const HServer::REQUEST_PROTO::DONE = "done";
int const HServer::IPC_CHANNEL::CONTROL = 0;
int const HServer::IPC_CHANNEL::REQUEST = 1;

HServer::HServer( int connections_ )
	: _maxConnections( connections_ ),
	_socket(), _requests(), _handlers(),
	_worker( setup._maxWorkingThreads ),
	_dispatcher( connections_, 3600 * 1000 ) {
	M_PROLOG
	_socket[ IPC_CHANNEL::CONTROL ] = make_pointer<HSocket>(
		HSocket::socket_type_t( HSocket::TYPE::FILE ) | HSocket::TYPE::NONBLOCKING, connections_
	);
	_socket[ IPC_CHANNEL::REQUEST ] = make_pointer<HSocket>(
		HSocket::socket_type_t( HSocket::TYPE::FILE ) | HSocket::TYPE::NONBLOCKING, connections_
	);
	return;
	M_EPILOG
}

HServer::~HServer( void ) {
	OUT << brightred << "<<<hector>>>" << lightgray << " server finished." << endl;
}

int HServer::init_server( void ) {
	M_PROLOG
	init_sockets();
	_handlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::SHUTDOWN ] = &HServer::handler_shutdown;
	_handlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::RESTART ] = &HServer::handler_restart;
	_handlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::RELOAD ] = &HServer::handler_reload;
	_handlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::STATUS ] = &HServer::handler_status;
	_handlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::ENV ] = &HServer::handler_env;
	_handlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::COOKIE ] = &HServer::handler_cookie;
	_handlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::GET ] = &HServer::handler_get;
	_handlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::POST ] = &HServer::handler_post;
	_handlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::DONE ] = &HServer::handler_done;
	_dispatcher.register_file_descriptor_handler(
		_socket[ IPC_CHANNEL::CONTROL ],
		call( &HServer::handler_connection, this, _1 )
	);
	_dispatcher.register_file_descriptor_handler(
		_socket[ IPC_CHANNEL::REQUEST ],
		call( &HServer::handler_connection, this, _1 )
	);
	OUT << brightblue << "<<<hector>>>" << lightgray << " server started." << endl;
	return ( 0 );
	M_EPILOG
}

void HServer::init_sockets( void ) {
	M_PROLOG
	static char const* const REQ_SOCK_NAME = "/request.sock";
	static char const* const CTRL_SOCK_NAME = "/control.sock";
	HString reqSockPath( setup._socketRoot );
	HString ctrlSockPath( setup._socketRoot );
	reqSockPath += REQ_SOCK_NAME;
	ctrlSockPath += CTRL_SOCK_NAME;
	hcore::log( LOG_LEVEL::INFO ) << "Using `" << reqSockPath << "' as IPC request inteface." << endl;
	hcore::log( LOG_LEVEL::INFO ) << "Using `" << ctrlSockPath << "' as IPC control inteface." << endl;
	int err = 0;
	HUTF8String reqSockPathUtf8( reqSockPath );
	HUTF8String ctrlSockPathUtf8( ctrlSockPath );
	M_ENSURE( ( ! ( err = ::unlink( reqSockPathUtf8.c_str() ) ) ) || ( errno == ENOENT ) );
	M_ENSURE( ( ! ( err = ::unlink( ctrlSockPathUtf8.c_str() ) ) ) || ( errno == ENOENT ) );
	_socket[ IPC_CHANNEL::CONTROL ]->listen( ctrlSockPath );
	_socket[ IPC_CHANNEL::REQUEST ]->listen( reqSockPath );
	M_ENSURE( ! ::chmod( reqSockPathUtf8.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ) );
	M_ENSURE( ! ::chmod( ctrlSockPathUtf8.c_str(), S_IRUSR | S_IWUSR ) );
	return;
	M_EPILOG
}

void HServer::handler_connection( HIODispatcher::stream_t& stream_ ) {
	M_PROLOG
	IPC_CHANNEL::ipc_channel_t channel = _socket[ IPC_CHANNEL::CONTROL ] == stream_ ? IPC_CHANNEL::CONTROL : IPC_CHANNEL::REQUEST;
	HSocket::ptr_t client = _socket[ channel ]->accept();
	M_ASSERT( !! client );
	if ( _requests.get_size() < _maxConnections ) {
		if ( channel == IPC_CHANNEL::REQUEST ) {
			_requests.insert( make_pair( client.raw(), ORequest( client ) ) );
		}
		_dispatcher.register_file_descriptor_handler(
			client,
			call(
				channel == IPC_CHANNEL::REQUEST ? &HServer::handler_request : &HServer::handler_control,
				this,
				_1
			)
		);
	}
	OUT << green << "new connection" << lightgray << endl;
	return;
	M_EPILOG
}

void HServer::handler_request( HIODispatcher::stream_t& stream_ ) {
	handler_message( stream_, IPC_CHANNEL::REQUEST );
	return;
}

void HServer::handler_control( HIODispatcher::stream_t& stream_ ) {
	handler_message( stream_, IPC_CHANNEL::CONTROL );
	return;
}

void HServer::handler_message( HIODispatcher::stream_t& stream_, int ipcChannel_ ) {
	M_PROLOG
	HString message;
	if ( !! stream_ ) {
		int long nRead( 0 );
		if ( ( nRead = stream_->read_until( message ) ) > 0 ) {
			OUT << "<-" << message << endl;
			static HString command;
			static HString argument;
			int long sepIdx = message.find( ":" );
			command = message.left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
			argument = message.mid( sepIdx + 1 );
			int msgLength = static_cast<int>( command.get_length() );
			if ( msgLength < 1 ) {
				disconnect_client( ipcChannel_, stream_, _( "Malformed data." ) );
			} else {
				handlers_t::iterator it = _handlers[ ipcChannel_ ].find( command );
				if ( it != _handlers[ ipcChannel_ ].end() ) {
					( this->*it->second )( stream_, argument );
				} else {
					disconnect_client( ipcChannel_, stream_, _( "Unknown command." ) );
				}
			}
		} else if ( ! nRead ) {
			disconnect_client( ipcChannel_, stream_ );
		}
		/* else nRead < 0 => REPEAT */
	}
	return;
	M_EPILOG
}

void HServer::disconnect_client( IPC_CHANNEL::ipc_channel_t channel,
		yaal::hcore::HStreamInterface::ptr_t& client_,
		char const* const reason_ ) {
	M_PROLOG
	M_ASSERT( !! client_ );
	_dispatcher.unregister_file_descriptor_handler( client_ );
	if ( channel == IPC_CHANNEL::REQUEST ) {
		_requests.erase( client_.raw() );
	}
	OUT << "client closed connection";
	if ( reason_ ) {
		clog << " " << reason_;
	}
	clog << endl;
	return;
	M_EPILOG
}

void HServer::read_request( HStreamInterface::ptr_t& sock, ORequest::origin_t const& origin, yaal::hcore::HString const& string_ ) {
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = _requests.find( sock.raw() ) ) == _requests.end() ) {
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	} else {
		HString key;
		HString value;
		int long sepIdx = string_.find( "=" );
		if ( sepIdx != HString::npos ) {
			key = yaal::move( string_.left( sepIdx ) );
			value = yaal::move( string_.mid( sepIdx + 1 ) );
			if ( origin == ORequest::ORIGIN::POST ) {
				value = HTTP::decode( value );
			}
		} else {
			key = string_;
		}
		key.trim_left().trim_right();
		value.trim_left().trim_right();
		reqIt->second.update( key, value, origin );
	}
	return;
	M_EPILOG
}

void HServer::handler_env( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& env_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::ENV, env_ );
	M_EPILOG
}

void HServer::handler_cookie( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& cookie_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::JAR, cookie_ );
	M_EPILOG
}

void HServer::handler_get( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& gET_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::GET, gET_ );
	M_EPILOG
}

void HServer::handler_post( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& pOST_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::POST, pOST_ );
	M_EPILOG
}

void HServer::handler_done( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& ) {
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = _requests.find( sock.raw() ) ) == _requests.end() )
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	else
		service_request( reqIt->second );
	M_EPILOG
}

void HServer::handler_shutdown( HStreamInterface::ptr_t&, yaal::hcore::HString const& ) {
	M_PROLOG
	_dispatcher.stop();
	return;
	M_EPILOG
}

void HServer::handler_restart( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& app ) {
	M_PROLOG
	_worker.schedule_task( HWorkFlow::SCHEDULE_POLICY::LAZY, call( &HServer::do_restart, this, sock, app ) );
	M_EPILOG
}

void HServer::handler_reload( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& app ) {
	M_PROLOG
	_worker.schedule_task( HWorkFlow::SCHEDULE_POLICY::LAZY, call( &HServer::do_reload, this, sock, app ) );
	M_EPILOG
}

void HServer::handler_status( HStreamInterface::ptr_t& sock, yaal::hcore::HString const& ) {
	M_PROLOG
	do_status( sock );
	M_EPILOG
}

void HServer::service_request( ORequest& request_ ) {
	M_PROLOG
	do_service_request( request_ );
	M_EPILOG
}

void HServer::run( void ) {
	M_PROLOG
	_dispatcher.run();
	M_EPILOG
}

}

