/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.hxx - this file is integral part of `gameground' project.

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

#ifndef SERVER_HXX_INCLUDED
#define SERVER_HXX_INCLUDED

#include <yaal/tools/hiodispatcher.hxx>
#include <yaal/tools/hworkflow.hxx>

#include "orequest.hxx"

namespace hector {

class HServer {
public:
	typedef void ( HServer::* handler_t )( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	typedef yaal::hcore::HMap<yaal::hcore::HString, handler_t> handlers_t;
	typedef yaal::hcore::HMap<int, ORequest> requests_t;
	struct REQUEST_PROTO {
		static char const* const ENV;
		static char const* const COOKIE;
		static char const* const GET;
		static char const* const POST;
		static char const* const DONE;
	};
	struct CONTROL_PROTO {
		static char const* const SHUTDOWN;
		static char const* const RELOAD;
		static char const* const RESTART;
		static char const* const STATUS;
	};
	struct IPC_CHANNEL {
		typedef int ipc_channel_t;
		static int const CONTROL;
		static int const REQUEST;
		static int const COUNT = 2;
	};
protected:
	/*{*/
	int _maxConnections;
	yaal::hcore::HSocket::ptr_t _socket[ IPC_CHANNEL::COUNT ];
	requests_t _requests;
	handlers_t _handlers[ IPC_CHANNEL::COUNT ];
	yaal::tools::HWorkFlow _worker;
	yaal::tools::HIODispatcher _dispatcher;
	/*}*/
public:
	/*{*/
	HServer( int );
	virtual ~HServer( void );
	int init_server( void );
	void run( void );
	/*}*/
protected:
	/*{*/
	void init_sockets( void );
	void disconnect_client( IPC_CHANNEL::ipc_channel_t const&, yaal::hcore::HSocket::ptr_t&, char const* const = NULL );
	void handler_connection( int );
	void handler_message( int );
	void handler_shutdown( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_restart( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_status( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_env( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_cookie( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_get( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_post( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void handler_done( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	void read_request( yaal::hcore::HSocket::ptr_t&, ORequest::origin_t const&, yaal::hcore::HString const& );
	void service_request( ORequest& );
	virtual void do_service_request( ORequest& ) = 0;
	virtual void do_restart( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& ) = 0;
	virtual void do_status( yaal::hcore::HSocket::ptr_t& ) = 0;
	/*}*/
};

int main_server( void );

}

#endif /* not SERVER_HXX_INCLUDED */

