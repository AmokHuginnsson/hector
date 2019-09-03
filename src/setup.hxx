/* Read hector/LICENSE.md file for copyright and licensing information. */

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
	/* self-sufficient */
	static int const PATH_OFFSET = sizeof ( __FILE__ ) - sizeof ( "setup.hxx" );
	OSetup( void )
		: _quiet( false )
		, _verbose( false )
		, _shutdown( false )
		, _status( false )
		, _programName( nullptr )
		, _maxConnections( DEFAULT_MAX_CONNECTIONS )
		, _maxWorkingThreads( DEFAULT_MAX_WORKING_THREADS )
		, _socketWriteTimeout( DEFAULT_SOCKET_WRITE_TIMEOUT )
		, _sslPort( DEFAULT_SSL_PORT )
		, _logPath()
		, _dataDir()
		, _socketRoot( DEFAULT_SOCKET_ROOT )
		, _reload()
		, _restart() {
	}
	void test_setup( void );
private:
	OSetup ( OSetup const & );
	OSetup & operator = ( OSetup const & );
};

extern OSetup setup;

}

#endif /* SETUP_HXX_INCLUDED */
