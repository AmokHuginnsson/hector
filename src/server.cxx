/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.cxx - this file is integral part of `gameground' project.

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
#include <sys/stat.h>

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/ansi.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "server.hxx"

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
	M_ENSURE( ( ! ( err = ::unlink( reqSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	M_ENSURE( ( ! ( err = ::unlink( ctrlSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	_socket[ IPC_CHANNEL::CONTROL ]->listen( ctrlSockPath );
	_socket[ IPC_CHANNEL::REQUEST ]->listen( reqSockPath );
	M_ENSURE( ! ::chmod( reqSockPath.raw(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ) );
	M_ENSURE( ! ::chmod( ctrlSockPath.raw(), S_IRUSR | S_IWUSR ) );
	return;
	M_EPILOG
}

void HServer::handler_connection( HIODispatcher::stream_t& stream_ ) {
	M_PROLOG
	IPC_CHANNEL::ipc_channel_t channel = _socket[ IPC_CHANNEL::CONTROL ] == stream_ ? IPC_CHANNEL::CONTROL : IPC_CHANNEL::REQUEST;
	HSocket::ptr_t client = _socket[ channel ]->accept();
	M_ASSERT( !! client );
	int fd = client->get_file_descriptor();
	if ( _socket[ channel ]->get_client_count() >= _maxConnections ) {
		_socket[ channel ]->shutdown_client( fd );
	} else {
		if ( channel == IPC_CHANNEL::REQUEST ) {
			_requests[ fd ] = ORequest( client );
		}
		_dispatcher.register_file_descriptor_handler( client, call( &HServer::handler_message, this, _1 ) );
	}
	OUT << green << "new connection" << lightgray << endl;
	return;
	M_EPILOG
}

void HServer::handler_message( HIODispatcher::stream_t& stream_ ) {
	M_PROLOG
	HString message;
	IPC_CHANNEL::ipc_channel_t channel = IPC_CHANNEL::REQUEST;
	int fd( static_cast<int>( reinterpret_cast<int_native_t>( stream_->data() ) ) );
	HSocket::iterator clientIt( _socket[ channel ]->find( fd ) );
	HSocket::ptr_t client;
	if ( clientIt == _socket[ channel ]->end() ) {
		channel = IPC_CHANNEL::CONTROL;
		clientIt = _socket[ channel ]->find( fd );
	}
	if ( clientIt != _socket[ channel ]->end() ) {
		client = clientIt->second;
	}
	if ( !! client ) {
		int long nRead( 0 );
		if ( ( nRead = client->read_until( message ) ) > 0 ) {
			OUT << "<-" << message << endl;
			static HString command;
			static HString argument;
			int long sepIdx = message.find( ":" );
			command = message.left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
			argument = message.mid( sepIdx + 1 );
			int msgLength = static_cast<int>( command.get_length() );
			if ( msgLength < 1 )
				disconnect_client( channel, client, _( "Malformed data." ) );
			else {
				handlers_t::iterator it = _handlers[ channel ].find( command );
				if ( it != _handlers[ channel ].end() )
					( this->*it->second )( client, argument );
				else
					disconnect_client( channel, client, _( "Unknown command." ) );
			}
		} else if ( ! nRead )
			disconnect_client( channel, client );
		/* else nRead < 0 => REPEAT */
	}
	return;
	M_EPILOG
}

void HServer::disconnect_client( IPC_CHANNEL::ipc_channel_t const& channel,
		yaal::hcore::HSocket::ptr_t& client_,
		char const* const reason_ ) {
	M_PROLOG
	M_ASSERT( !! client_ );
	int fileDescriptor = client_->get_file_descriptor();
	_dispatcher.unregister_file_descriptor_handler( client_ );
	if ( channel == IPC_CHANNEL::REQUEST )
		_requests.erase( fileDescriptor );
	_socket[ channel ]->shutdown_client( fileDescriptor );
	OUT << "client closed connection";
	if ( reason_ )
		clog << " " << reason_;
	clog << endl;
	return;
	M_EPILOG
}

void HServer::read_request( HSocket::ptr_t& sock, ORequest::origin_t const& origin, yaal::hcore::HString const& string_ ) {
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = _requests.find( sock->get_file_descriptor() ) ) == _requests.end() ) {
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	} else {
		HString key;
		HString value;
		int long sepIdx = string_.find( "=" );
		if ( sepIdx != HString::npos ) {
			key = yaal::move( string_.left( sepIdx ) );
			value = yaal::move( string_.mid( sepIdx + 1 ) );
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

void HServer::handler_env( HSocket::ptr_t& sock, yaal::hcore::HString const& env_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::ENV, env_ );
	M_EPILOG
}

void HServer::handler_cookie( HSocket::ptr_t& sock, yaal::hcore::HString const& cookie_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::JAR, cookie_ );
	M_EPILOG
}

void HServer::handler_get( HSocket::ptr_t& sock, yaal::hcore::HString const& gET_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::GET, gET_ );
	M_EPILOG
}

void HServer::handler_post( HSocket::ptr_t& sock, yaal::hcore::HString const& pOST_ ) {
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::POST, pOST_ );
	M_EPILOG
}

void HServer::handler_done( HSocket::ptr_t& sock, yaal::hcore::HString const& ) {
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = _requests.find( sock->get_file_descriptor() ) ) == _requests.end() )
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	else
		service_request( reqIt->second );
	M_EPILOG
}

void HServer::handler_shutdown( HSocket::ptr_t&, yaal::hcore::HString const& ) {
	M_PROLOG
	_dispatcher.stop();
	return;
	M_EPILOG
}

void HServer::handler_restart( HSocket::ptr_t& sock, yaal::hcore::HString const& app ) {
	M_PROLOG
	_worker.schedule_task( call( &HServer::do_restart, this, sock, app ) );
	M_EPILOG
}

void HServer::handler_reload( HSocket::ptr_t& sock, yaal::hcore::HString const& app ) {
	M_PROLOG
	_worker.schedule_task( call( &HServer::do_reload, this, sock, app ) );
	M_EPILOG
}

void HServer::handler_status( HSocket::ptr_t& sock, yaal::hcore::HString const& ) {
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

