/* Read hector/LICENSE.md file for copyright and licensing information. */

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
	void disconnect_client( IPC_CHANNEL::ipc_channel_t, yaal::hcore::HStreamInterface::ptr_t&, char const* const = nullptr );
	void handler_connection( yaal::tools::HIODispatcher::stream_t&, yaal::hcore::system::IO_EVENT_TYPE );
	void handler_message( yaal::tools::HIODispatcher::stream_t&, int );
	void handler_request( yaal::tools::HIODispatcher::stream_t&, yaal::hcore::system::IO_EVENT_TYPE );
	void handler_control( yaal::tools::HIODispatcher::stream_t&, yaal::hcore::system::IO_EVENT_TYPE );
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

