/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.hxx - this file is integral part of `gameground' project.

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

#ifndef SERVER_HXX_INCLUDED
#define SERVER_HXX_INCLUDED

#include <yaal/tools/hiodispatcher.hxx>
#include <yaal/tools/hworkflow.hxx>

#include "orequest.hxx"

namespace hector {

class HServer {
public:
	typedef void ( HServer::* handler_t )( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	typedef yaal::hcore::HMap<yaal::hcore::HString, handler_t> handlers_t;
	typedef yaal::hcore::HMap<yaal::hcore::HStreamInterface*, ORequest> requests_t;
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
	void disconnect_client( IPC_CHANNEL::ipc_channel_t, yaal::hcore::HStreamInterface::ptr_t&, char const* const = NULL );
	void handler_connection( yaal::tools::HIODispatcher::stream_t& );
	void handler_message( yaal::tools::HIODispatcher::stream_t&, int );
	void handler_request( yaal::tools::HIODispatcher::stream_t& );
	void handler_control( yaal::tools::HIODispatcher::stream_t& );
	void handler_shutdown( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_restart( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_reload( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_status( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_env( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_cookie( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_get( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_post( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void handler_done( yaal::hcore::HStreamInterface::ptr_t&, yaal::hcore::HString const& );
	void read_request( yaal::hcore::HStreamInterface::ptr_t&, ORequest::origin_t const&, yaal::hcore::HString const& );
	void service_request( ORequest& );
	virtual void do_service_request( ORequest& ) = 0;
	virtual void do_restart( yaal::hcore::HStreamInterface::ptr_t, yaal::hcore::HString const& ) = 0;
	virtual void do_reload( yaal::hcore::HStreamInterface::ptr_t, yaal::hcore::HString const& ) = 0;
	virtual void do_status( yaal::hcore::HStreamInterface::ptr_t& ) = 0;
	/*}*/
};

int main_server( void );

}

#endif /* not SERVER_HXX_INCLUDED */

