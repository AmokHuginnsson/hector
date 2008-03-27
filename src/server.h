/*
---           `gameground' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	server.h - this file is integral part of `gameground' project.

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

#ifndef __SERVER_H
#define __SERVER_H

#include <yaal/tools/hprocess.h>

#include "orequest.h"

namespace hector
{

class HServer : public yaal::tools::HProcess
	{
public:
	typedef void ( HServer::* handler_t )( ORequest&, yaal::hcore::HString const& );
	typedef yaal::hcore::HMap<yaal::hcore::HString, handler_t> handlers_t;
	typedef yaal::hcore::HMap<int, ORequest> requests_t;
	struct REQUEST_PROTO
		{
		static char const* const ENV;
		static char const* const COOKIE;
		static char const* const GET;
		static char const* const POST;
		static char const* const DONE;
		};
protected:
	/*{*/
	int f_iMaxConnections;
	yaal::hcore::HSocket f_oSocket;
	requests_t f_oRequests;
	handlers_t f_oHandlers;
	/*}*/
public:
	/*{*/
	HServer( int );
	virtual ~HServer( void );
	int init_server( char const* const );
	using yaal::tools::HProcess::run;
	/*}*/
protected:
	/*{*/
	void disconnect_client( yaal::hcore::HSocket::ptr_t&, char const* const = NULL );
	int handler_connection( int );
	int handler_message( int );
	void handler_env( ORequest&, yaal::hcore::HString const& );
	void handler_cookie( ORequest&, yaal::hcore::HString const& );
	void handler_get( ORequest&, yaal::hcore::HString const& );
	void handler_post( ORequest&, yaal::hcore::HString const& );
	void handler_done( ORequest&, yaal::hcore::HString const& );
	void read_request( ORequest&, ORequest::ORIGIN::origin_t const&, yaal::hcore::HString const& );
	void service_request( ORequest& );
	virtual void do_service_request( ORequest& ) = 0;
	/*}*/
	};

int main_server( void );

}

#endif /* not __SERVER_H */

