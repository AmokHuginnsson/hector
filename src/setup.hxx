/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

	setup.hxx - this file is integral part of `hector' project.

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

#ifndef SETUP_HXX_INCLUDED
#define SETUP_HXX_INCLUDED

#include <libintl.h>
#include <yaal/yaal.hxx>
#include <iostream>

#include "config.hxx"

#define out ( clog << __FILE__ + OSetup::PATH_OFFSET << ":" << __LINE__ << ": " )

namespace hector
{

inline std::ostream& operator << ( std::ostream& o, yaal::hcore::HString const& s )
	{ return ( o << s.raw() ); }

struct OSetup
	{
	static int const DEFAULT_MAX_CONNECTIONS = 2;
	static int const DEFAULT_MAX_WORKING_THREADS = 2;
	static int const DEFAULT_SOCKET_WRITE_TIMEOUT = 5; /* seconds */
	static char const* const DEFAULT_SOCKET_ROOT;
	bool _quiet;			/* --quiet, --silent */
	bool _verbose;		/* --verbose */
	bool _shutdown;
	bool _status;
	char* _programName;
	int _maxConnections;
	int _maxWorkingThreads;
	int _socketWriteTimeout;
	yaal::hcore::HString _logPath;
	yaal::hcore::HString _dataDir;
	yaal::hcore::HString _socketRoot;
	yaal::hcore::HString _reload;
	yaal::hcore::HString _restart;
	yaal::hcore::HString _databaseName;
	yaal::hcore::HString _databaseLogin;
	yaal::hcore::HString _databasePassword;
	/* self-sufficient */
	static int const PATH_OFFSET = sizeof ( __FILE__ ) - sizeof ( "setup.hxx" );
	OSetup( void )
		: _quiet( false ), _verbose( false ),
		_shutdown( false ),
		_status( false ), _programName( NULL ),
		_maxConnections( DEFAULT_MAX_CONNECTIONS ),
		_maxWorkingThreads( DEFAULT_MAX_WORKING_THREADS ),
		_socketWriteTimeout( DEFAULT_SOCKET_WRITE_TIMEOUT ),
		_logPath(), _dataDir(), _socketRoot( DEFAULT_SOCKET_ROOT ),
		_reload(), _restart(),
		_databaseName(), _databaseLogin(), _databasePassword() {}
	void test_setup( void );
private:
	OSetup ( OSetup const & );
	OSetup & operator = ( OSetup const & );
	};

extern OSetup setup;

}

#endif /* SETUP_HXX_INCLUDED */
