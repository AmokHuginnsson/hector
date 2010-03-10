/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.cxx - this file is integral part of `gameground' project.

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

#include <sys/stat.h>

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "server.hxx"

#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

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

HServer::HServer( int a_iConnections )
	: HProcess( a_iConnections ), f_iMaxConnections( a_iConnections ),
	f_oSocket(), f_oRequests(), f_oHandlers(), f_oWorker( setup.f_iMaxWorkingThreads )
	{
	M_PROLOG
	f_oSocket[ IPC_CHANNEL::CONTROL ] = HSocket::ptr_t(
			new HSocket( HSocket::socket_type_t( HSocket::TYPE::FILE ) | HSocket::TYPE::NONBLOCKING, a_iConnections ) );
	f_oSocket[ IPC_CHANNEL::REQUEST ] = HSocket::ptr_t(
			new HSocket( HSocket::socket_type_t( HSocket::TYPE::FILE ) | HSocket::TYPE::NONBLOCKING, a_iConnections ) );
	return;
	M_EPILOG
	}

HServer::~HServer( void )
	{
	out << brightred << "<<<hector>>>" << lightgray << " server finished." << endl;
	}

int HServer::init_server( void )
	{
	M_PROLOG
	init_sockets();
	f_oHandlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::SHUTDOWN ] = &HServer::handler_shutdown;
	f_oHandlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::RESTART ] = &HServer::handler_restart;
	f_oHandlers[ IPC_CHANNEL::CONTROL ][ CONTROL_PROTO::STATUS ] = &HServer::handler_status;
	f_oHandlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::ENV ] = &HServer::handler_env;
	f_oHandlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::COOKIE ] = &HServer::handler_cookie;
	f_oHandlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::GET ] = &HServer::handler_get;
	f_oHandlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::POST ] = &HServer::handler_post;
	f_oHandlers[ IPC_CHANNEL::REQUEST ][ REQUEST_PROTO::DONE ] = &HServer::handler_done;
	register_file_descriptor_handler( f_oSocket[ IPC_CHANNEL::CONTROL ]->get_file_descriptor(), &HServer::handler_connection );
	register_file_descriptor_handler( f_oSocket[ IPC_CHANNEL::REQUEST ]->get_file_descriptor(), &HServer::handler_connection );
	HProcess::init ( 3600 );
	out << brightblue << "<<<hector>>>" << lightgray << " server started." << endl;
	return ( 0 );
	M_EPILOG
	}

void HServer::init_sockets( void )
	{
	M_PROLOG
	static char const* const REQ_SOCK_NAME = "/request.sock";
	static char const* const CTRL_SOCK_NAME = "/control.sock";
	HString reqSockPath( setup.f_oSocketRoot );
	HString ctrlSockPath( setup.f_oSocketRoot );
	reqSockPath += REQ_SOCK_NAME;
	ctrlSockPath += CTRL_SOCK_NAME;
	hcore::log( LOG_TYPE::INFO ) << "Using `" << reqSockPath << "' as IPC request inteface." << endl;
	hcore::log( LOG_TYPE::INFO ) << "Using `" << ctrlSockPath << "' as IPC control inteface." << endl;
	int err = 0;
	M_ENSURE( ( ! ( err = ::unlink( reqSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	M_ENSURE( ( ! ( err = ::unlink( ctrlSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	f_oSocket[ IPC_CHANNEL::CONTROL ]->listen( ctrlSockPath );
	f_oSocket[ IPC_CHANNEL::REQUEST ]->listen( reqSockPath );
	M_ENSURE( ! ::chmod( reqSockPath.raw(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ) );
	M_ENSURE( ! ::chmod( ctrlSockPath.raw(), S_IRUSR | S_IWUSR ) );
	return;
	M_EPILOG
	}

int HServer::handler_connection( int msgFd )
	{
	M_PROLOG
	IPC_CHANNEL::ipc_channel_t channel = f_oSocket[ IPC_CHANNEL::CONTROL ]->get_file_descriptor() == msgFd ? IPC_CHANNEL::CONTROL : IPC_CHANNEL::REQUEST;
	HSocket::ptr_t l_oClient = f_oSocket[ channel ]->accept();
	M_ASSERT( !! l_oClient );
	int fd = l_oClient->get_file_descriptor();
	if ( f_oSocket[ channel ]->get_client_count() >= f_iMaxConnections )
		f_oSocket[ channel ]->shutdown_client( fd );
	else
		{
		if ( channel == IPC_CHANNEL::REQUEST )
			f_oRequests[ fd ] = ORequest( l_oClient );
		register_file_descriptor_handler( fd, &HServer::handler_message );
		}
	out << green << "new connection" << lightgray << endl;
	return ( 0 );
	M_EPILOG
	}

int HServer::handler_message( int a_iFileDescriptor )
	{
	M_PROLOG
	HString l_oMessage;
	IPC_CHANNEL::ipc_channel_t channel = IPC_CHANNEL::REQUEST;
	HSocket::ptr_t l_oClient = f_oSocket[ channel ]->get_client( a_iFileDescriptor );
	if ( ! l_oClient )
		{
		channel = IPC_CHANNEL::CONTROL;
		l_oClient = f_oSocket[ channel ]->get_client( a_iFileDescriptor );
		}
	if ( !! l_oClient )
		{
		int long nRead( 0 );
		if ( ( nRead = l_oClient->read_until( l_oMessage ) ) > 0 )
			{
			out << "<-" << l_oMessage << endl;
			static HString l_oCommand;
			static HString l_oArgument;
			int long sepIdx = l_oMessage.find( ":" );
			l_oCommand = l_oMessage.left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
			l_oArgument = l_oMessage.mid( sepIdx + 1 );
			int l_iMsgLength = static_cast<int>( l_oCommand.get_length() );
			if ( l_iMsgLength < 1 )
				disconnect_client( channel, l_oClient, _( "Malformed data." ) );
			else
				{
				handlers_t::iterator it = f_oHandlers[ channel ].find( l_oCommand );
				if ( it != f_oHandlers[ channel ].end() )
					( this->*it->second )( l_oClient, l_oArgument );
				else
					disconnect_client( channel, l_oClient, _( "Unknown command." ) );
				}
			}
		else if ( ! nRead )
			disconnect_client( channel, l_oClient );
		/* else nRead < 0 => REPEAT */
		}
	return ( 0 );
	M_EPILOG
	}

void HServer::disconnect_client( IPC_CHANNEL::ipc_channel_t const& channel,
		yaal::hcore::HSocket::ptr_t& a_oClient,
		char const* const a_pcReason )
	{
	M_PROLOG
	M_ASSERT( !! a_oClient );
	int l_iFileDescriptor = a_oClient->get_file_descriptor();
	unregister_file_descriptor_handler( l_iFileDescriptor );
	if ( channel == IPC_CHANNEL::REQUEST )
		f_oRequests.remove( l_iFileDescriptor );
	f_oSocket[ channel ]->shutdown_client( l_iFileDescriptor );
	out << "client closed connection";
	if ( a_pcReason )
		cout << " " << a_pcReason;
	cout << endl;
	return;
	M_EPILOG
	}

void HServer::read_request( HSocket::ptr_t& sock, ORequest::origin_t const& origin, yaal::hcore::HString const& a_oString )
	{
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = f_oRequests.find( sock->get_file_descriptor() ) ) == f_oRequests.end() )
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	else
		{
		static HString key;
		static HString value;
		int long sepIdx = a_oString.find( "=" );
		key = a_oString.left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
		value = a_oString.mid( sepIdx + 1 );
		key.trim_left().trim_right();
		value.trim_left().trim_right();
		reqIt->second.update( key, value, origin );
		}
	return;
	M_EPILOG
	}

void HServer::handler_env( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oEnv )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::ENV, a_oEnv );
	M_EPILOG
	}

void HServer::handler_cookie( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oCookie )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::JAR, a_oCookie );
	M_EPILOG
	}

void HServer::handler_get( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oGET )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::GET, a_oGET );
	M_EPILOG
	}

void HServer::handler_post( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oPOST )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::POST, a_oPOST );
	M_EPILOG
	}

void HServer::handler_done( HSocket::ptr_t& sock, yaal::hcore::HString const& )
	{
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = f_oRequests.find( sock->get_file_descriptor() ) ) == f_oRequests.end() )
		disconnect_client( IPC_CHANNEL::REQUEST, sock );
	else
		service_request( reqIt->second );
	M_EPILOG
	}

void HServer::handler_shutdown( HSocket::ptr_t&, yaal::hcore::HString const& )
	{
	f_bLoop = false;
	}

void HServer::handler_restart( HSocket::ptr_t& sock, yaal::hcore::HString const& app )
	{
	M_PROLOG
	f_oWorker.push_task( bound_call( &HServer::do_restart, this, sock, app ) );
	M_EPILOG
	}

void HServer::handler_status( HSocket::ptr_t& sock, yaal::hcore::HString const& )
	{
	M_PROLOG
	do_status( sock );
	M_EPILOG
	}

void HServer::service_request( ORequest& a_roRequest )
	{
	M_PROLOG
	do_service_request( a_roRequest );
	M_EPILOG
	}

}

