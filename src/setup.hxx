/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

  setup.hxx - this file is integral part of `hector' project.

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

#ifndef SETUP_HXX_INCLUDED
#define SETUP_HXX_INCLUDED

#include <libintl.h>
#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hfile.hxx>

#include "config.hxx"

#define OUT ( yaal::hcore::clog << __FILE__ + OSetup::PATH_OFFSET << ":" << __LINE__ << ": " )

namespace hector {

struct OSetup {
	static int const DEFAULT_MAX_CONNECTIONS = 2;
	static int const DEFAULT_MAX_WORKING_THREADS = 2;
	static int const DEFAULT_SOCKET_WRITE_TIMEOUT = 5; /* seconds */
	static int const DEFAULT_SSL_PORT = 443;
	static char const* const DEFAULT_SOCKET_ROOT;
	bool _quiet;			/* --quiet, --silent */
	bool _verbose;		/* --verbose */
	bool _shutdown;
	bool _status;
	char* _programName;
	int _maxConnections;
	int _maxWorkingThreads;
	int _socketWriteTimeout;
	int _sslPort;
	yaal::hcore::HString _logPath;
	yaal::hcore::HString _dataDir;
	yaal::hcore::HString _socketRoot;
	yaal::hcore::HString _reload;
	yaal::hcore::HString _restart;
	yaal::hcore::HString _dsn;
	yaal::hcore::HString _tableUser;
	yaal::hcore::HString _columnLogin;
	yaal::hcore::HString _columnPassword;
	yaal::hcore::HString _authQuery;
	/* self-sufficient */
	static int const PATH_OFFSET = sizeof ( __FILE__ ) - sizeof ( "setup.hxx" );
	OSetup( void )
		: _quiet( false )
		, _verbose( false )
		, _shutdown( false )
		, _status( false )
		, _programName( NULL )
		, _maxConnections( DEFAULT_MAX_CONNECTIONS )
		, _maxWorkingThreads( DEFAULT_MAX_WORKING_THREADS )
		, _socketWriteTimeout( DEFAULT_SOCKET_WRITE_TIMEOUT )
		, _sslPort( DEFAULT_SSL_PORT )
		, _logPath()
		, _dataDir()
		, _socketRoot( DEFAULT_SOCKET_ROOT )
		, _reload()
		, _restart()
		, _dsn()
		, _tableUser()
		, _columnLogin()
		, _columnPassword()
		, _authQuery() {
	}
	void test_setup( void );
private:
	OSetup ( OSetup const & );
	OSetup & operator = ( OSetup const & );
};

extern OSetup setup;

}

#endif /* SETUP_HXX_INCLUDED */
