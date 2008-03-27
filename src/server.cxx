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

#include <sys/wait.h>
#include <iostream>

#include <yaal/yaal.h>
M_VCSID ( "$Id$" )
#include "server.h"

#include "setup.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

char const* const HServer::REQUEST_PROTO::ENV = "env";
char const* const HServer::REQUEST_PROTO::COOKIE = "cookie";
char const* const HServer::REQUEST_PROTO::GET = "get";
char const* const HServer::REQUEST_PROTO::POST = "post";
char const* const HServer::REQUEST_PROTO::DONE = "done";

HServer::HServer( int a_iConnections )
	: HProcess( a_iConnections ), f_iMaxConnections( a_iConnections ),
	f_oSocket( HSocket::TYPE::D_FILE | HSocket::TYPE::D_NONBLOCKING, a_iConnections ),
	f_oRequests(), f_oHandlers()
	{
	M_PROLOG
	return;
	M_EPILOG
	}

HServer::~HServer( void )
	{
	out << brightred << "<<<hector>>>" << lightgray << " server finished." << endl;
	}

int HServer::init_server( char const* const a_pcPath )
	{
	M_PROLOG
	f_oSocket.listen ( a_pcPath );
	f_oHandlers[ REQUEST_PROTO::ENV ] = &HServer::handler_env;
	f_oHandlers[ REQUEST_PROTO::COOKIE ] = &HServer::handler_cookie;
	f_oHandlers[ REQUEST_PROTO::GET ] = &HServer::handler_get;
	f_oHandlers[ REQUEST_PROTO::POST ] = &HServer::handler_post;
	f_oHandlers[ REQUEST_PROTO::DONE ] = &HServer::handler_done;
	register_file_descriptor_handler( f_oSocket.get_file_descriptor(), &HServer::handler_connection );
	HProcess::init ( 3600 );
	out << brightblue << "<<<hector>>>" << lightgray << " server started." << endl;
	return ( 0 );
	M_EPILOG
	}

int HServer::handler_connection( int )
	{
	M_PROLOG
	HSocket::ptr_t l_oClient = f_oSocket.accept();
	M_ASSERT( !! l_oClient );
	if ( f_oSocket.get_client_count() >= f_iMaxConnections )
		{
		cout << "WHOA !!!" << endl;
		f_oSocket.shutdown_client( l_oClient->get_file_descriptor() );
		}
	else
		{
		int fd = l_oClient->get_file_descriptor();
		f_oRequests.insert( fd, ORequest() );
		register_file_descriptor_handler( fd, &HServer::handler_message );
		}
	out << green << "new connection" << lightgray << endl;
	return ( 0 );
	M_EPILOG
	}

int HServer::handler_message( int a_iFileDescriptor )
	{
	M_PROLOG
	int l_iMsgLength = 0;
	out << a_iFileDescriptor << endl;
	HString l_oMessage;
	HSocket::ptr_t l_oClient = f_oSocket.get_client( a_iFileDescriptor );
	requests_t::iterator reqIt;
	if ( !! l_oClient )
		{
		if ( ( reqIt = f_oRequests.find( a_iFileDescriptor ) ) == f_oRequests.end() )
			disconnect_client( l_oClient );
		else if ( ( l_iMsgLength = l_oClient->read_until( l_oMessage ) ) > 0 )
			{
			out << "<-" << static_cast<char const* const>( l_oMessage ) << endl;
			static HString l_oCommand;
			static HString l_oArgument;
			l_oCommand = l_oMessage.split( ":", 0 );
			l_oArgument = l_oMessage.mid( l_oCommand.get_length() + 1 );
			l_iMsgLength = l_oCommand.get_length();
			if ( l_iMsgLength < 1 )
				disconnect_client( l_oClient, _( "Malformed data." ) );
			else
				{
				handlers_t::iterator it = f_oHandlers.find( l_oCommand );
				if ( it != f_oHandlers.end() )
					( this->*it->second )( reqIt->second, l_oArgument );
				else
					disconnect_client( l_oClient, _( "Unknown command." ) );
				}
			}
		else if ( l_iMsgLength == 0 )
			disconnect_client( l_oClient );
		}
	return ( 0 );
	M_EPILOG
	}

void HServer::disconnect_client( yaal::hcore::HSocket::ptr_t& a_oClient,
		char const* const a_pcReason )
	{
	M_PROLOG
	M_ASSERT( !! a_oClient );
	int l_iFileDescriptor = a_oClient->get_file_descriptor();
	unregister_file_descriptor_handler( l_iFileDescriptor );
	f_oSocket.shutdown_client( l_iFileDescriptor );
	f_oRequests.remove( l_iFileDescriptor );
	out << "client closed connection";
	if ( a_pcReason )
		cout << " " << a_pcReason;
	cout << endl;
	return;
	M_EPILOG
	}

void HServer::read_request( ORequest::dictionary_t& dict, yaal::hcore::HString const& a_oString )
	{
	static HString key;
	key = a_oString.split( "=", 0 );
	key.trim_left().trim_right();
	static HString value;
	value = a_oString.split( "=", 1 );
	value.trim_left().trim_right();
	dict[ key ] = value;
	}

void HServer::handler_env( ORequest& a_roRequest, yaal::hcore::HString const& a_oEnv )
	{
	read_request( *a_roRequest.f_oEnvironment, a_oEnv );
	}

void HServer::handler_cookie( ORequest& a_roRequest, yaal::hcore::HString const& a_oCookie )
	{
	read_request( *a_roRequest.f_oCookies, a_oCookie );
	}

void HServer::handler_get( ORequest& a_roRequest, yaal::hcore::HString const& a_oGET )
	{
	read_request( *a_roRequest.f_oGET, a_oGET );
	}

void HServer::handler_post( ORequest& a_roRequest, yaal::hcore::HString const& a_oPOST )
	{
	read_request( *a_roRequest.f_oPOST, a_oPOST );
	}

void HServer::handler_done( ORequest&, yaal::hcore::HString const& )
	{
	}

}

