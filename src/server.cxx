/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.cxx - this file is integral part of `gameground' project.

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

#include <sys/stat.h>

#include <iostream>

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "server.hxx"

#include "setup.hxx"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

char const* const HServer::CONTROL_PROTO::SHUTDOWN = "shutdown";
char const* const HServer::CONTROL_PROTO::RELOAD = "reload";
char const* const HServer::CONTROL_PROTO::STATUS = "status";
char const* const HServer::REQUEST_PROTO::ENV = "env";
char const* const HServer::REQUEST_PROTO::COOKIE = "cookie";
char const* const HServer::REQUEST_PROTO::GET = "get";
char const* const HServer::REQUEST_PROTO::POST = "post";
char const* const HServer::REQUEST_PROTO::DONE = "done";
int const HServer::IPC_CHANNEL::D_CONTROL = 0;
int const HServer::IPC_CHANNEL::D_REQUEST = 1;

HServer::HServer( int a_iConnections )
	: HProcess( a_iConnections ), f_iMaxConnections( a_iConnections ),
	f_oSocket(), f_oRequests(), f_oHandlers()
	{
	M_PROLOG
	f_oSocket[ IPC_CHANNEL::D_CONTROL ] = HSocket::ptr_t( new HSocket( HSocket::TYPE::D_FILE | HSocket::TYPE::D_NONBLOCKING, a_iConnections ) );
	f_oSocket[ IPC_CHANNEL::D_REQUEST ] = HSocket::ptr_t( new HSocket( HSocket::TYPE::D_FILE | HSocket::TYPE::D_NONBLOCKING, a_iConnections ) );
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
	f_oHandlers[ IPC_CHANNEL::D_CONTROL ][ CONTROL_PROTO::SHUTDOWN ] = &HServer::handler_shutdown;
	f_oHandlers[ IPC_CHANNEL::D_CONTROL ][ CONTROL_PROTO::RELOAD ] = &HServer::handler_reload;
	f_oHandlers[ IPC_CHANNEL::D_CONTROL ][ CONTROL_PROTO::STATUS ] = &HServer::handler_status;
	f_oHandlers[ IPC_CHANNEL::D_REQUEST ][ REQUEST_PROTO::ENV ] = &HServer::handler_env;
	f_oHandlers[ IPC_CHANNEL::D_REQUEST ][ REQUEST_PROTO::COOKIE ] = &HServer::handler_cookie;
	f_oHandlers[ IPC_CHANNEL::D_REQUEST ][ REQUEST_PROTO::GET ] = &HServer::handler_get;
	f_oHandlers[ IPC_CHANNEL::D_REQUEST ][ REQUEST_PROTO::POST ] = &HServer::handler_post;
	f_oHandlers[ IPC_CHANNEL::D_REQUEST ][ REQUEST_PROTO::DONE ] = &HServer::handler_done;
	register_file_descriptor_handler( f_oSocket[ IPC_CHANNEL::D_CONTROL ]->get_file_descriptor(), &HServer::handler_connection );
	register_file_descriptor_handler( f_oSocket[ IPC_CHANNEL::D_REQUEST ]->get_file_descriptor(), &HServer::handler_connection );
	HProcess::init ( 3600 );
	out << brightblue << "<<<hector>>>" << lightgray << " server started." << endl;
	return ( 0 );
	M_EPILOG
	}

void HServer::init_sockets( void )
	{
	M_PROLOG
	static char const* const D_REQ_SOCK_NAME = "/request.sock";
	static char const* const D_CTRL_SOCK_NAME = "/control.sock";
	HString reqSockPath( setup.f_oSocketRoot );
	HString ctrlSockPath( setup.f_oSocketRoot );
	reqSockPath += D_REQ_SOCK_NAME;
	ctrlSockPath += D_CTRL_SOCK_NAME;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << reqSockPath << "' as IPC request inteface." << endl;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << ctrlSockPath << "' as IPC control inteface." << endl;
	int err = 0;
	M_ENSURE( ( ! ( err = ::unlink( reqSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	M_ENSURE( ( ! ( err = ::unlink( ctrlSockPath.raw() ) ) ) || ( errno == ENOENT ) );
	f_oSocket[ IPC_CHANNEL::D_CONTROL ]->listen( ctrlSockPath );
	f_oSocket[ IPC_CHANNEL::D_REQUEST ]->listen( reqSockPath );
	M_ENSURE( ! ::chmod( reqSockPath.raw(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH ) );
	M_ENSURE( ! ::chmod( ctrlSockPath.raw(), S_IRUSR | S_IWUSR ) );
	return;
	M_EPILOG
	}

int HServer::handler_connection( int msgFd )
	{
	M_PROLOG
	IPC_CHANNEL::ipc_channel_t channel = f_oSocket[ IPC_CHANNEL::D_CONTROL ]->get_file_descriptor() == msgFd ? IPC_CHANNEL::D_CONTROL : IPC_CHANNEL::D_REQUEST;
	HSocket::ptr_t l_oClient = f_oSocket[ channel ]->accept();
	M_ASSERT( !! l_oClient );
	int fd = l_oClient->get_file_descriptor();
	if ( f_oSocket[ channel ]->get_client_count() >= f_iMaxConnections )
		f_oSocket[ channel ]->shutdown_client( fd );
	else
		{
		if ( channel == IPC_CHANNEL::D_REQUEST )
			f_oRequests.insert( fd, ORequest( l_oClient ) );
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
	IPC_CHANNEL::ipc_channel_t channel = IPC_CHANNEL::D_REQUEST;
	HSocket::ptr_t l_oClient = f_oSocket[ channel ]->get_client( a_iFileDescriptor );
	if ( ! l_oClient )
		{
		channel = IPC_CHANNEL::D_CONTROL;
		l_oClient = f_oSocket[ channel ]->get_client( a_iFileDescriptor );
		}
	if ( !! l_oClient )
		{
		HSocket::HStreamInterface::STATUS const* status = NULL;
		if ( ( status = &l_oClient->read_until( l_oMessage ) )->code == HSocket::HStreamInterface::STATUS::D_OK )
			{
			out << "<-" << l_oMessage << endl;
			static HString l_oCommand;
			static HString l_oArgument;
			l_oCommand = l_oMessage.split( ":", 0 );
			l_oArgument = l_oMessage.mid( l_oCommand.get_length() + 1 );
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
		else if ( status->code == HSocket::HStreamInterface::STATUS::D_ERROR )
			disconnect_client( channel, l_oClient );
		/* else status->code == HSocket::HStreamInterface::STATUS::D_REPEAT */
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
	if ( channel == IPC_CHANNEL::D_REQUEST )
		f_oRequests.remove( l_iFileDescriptor );
	f_oSocket[ channel ]->shutdown_client( l_iFileDescriptor );
	out << "client closed connection";
	if ( a_pcReason )
		cout << " " << a_pcReason;
	cout << endl;
	return;
	M_EPILOG
	}

void HServer::read_request( HSocket::ptr_t& sock, ORequest::ORIGIN::origin_t const& origin, yaal::hcore::HString const& a_oString )
	{
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = f_oRequests.find( sock->get_file_descriptor() ) ) == f_oRequests.end() )
		disconnect_client( IPC_CHANNEL::D_REQUEST, sock );
	else
		{
		static HString key;
		key = a_oString.split( "=", 0 );
		key.trim_left().trim_right();
		static HString value;
		value = a_oString.split( "=", 1 );
		value.trim_left().trim_right();
		reqIt->second.update( key, value, origin );
		}
	return;
	M_EPILOG
	}

void HServer::handler_env( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oEnv )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::D_ENV, a_oEnv );
	M_EPILOG
	}

void HServer::handler_cookie( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oCookie )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::D_JAR, a_oCookie );
	M_EPILOG
	}

void HServer::handler_get( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oGET )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::D_GET, a_oGET );
	M_EPILOG
	}

void HServer::handler_post( HSocket::ptr_t& sock, yaal::hcore::HString const& a_oPOST )
	{
	M_PROLOG
	read_request( sock, ORequest::ORIGIN::D_POST, a_oPOST );
	M_EPILOG
	}

void HServer::handler_done( HSocket::ptr_t& sock, yaal::hcore::HString const& )
	{
	M_PROLOG
	requests_t::iterator reqIt;
	if ( ( reqIt = f_oRequests.find( sock->get_file_descriptor() ) ) == f_oRequests.end() )
		disconnect_client( IPC_CHANNEL::D_REQUEST, sock );
	else
		service_request( reqIt->second );
	M_EPILOG
	}

void HServer::handler_shutdown( HSocket::ptr_t&, yaal::hcore::HString const& )
	{
	f_bLoop = false;
	}

void HServer::handler_reload( HSocket::ptr_t& sock, yaal::hcore::HString const& app )
	{
	do_reload( sock, app );
	}

void HServer::handler_status( HSocket::ptr_t& sock, yaal::hcore::HString const& )
	{
	do_status( sock );
	}

void HServer::service_request( ORequest& a_roRequest )
	{
	M_PROLOG
	do_service_request( a_roRequest );
	M_EPILOG
	}

}

