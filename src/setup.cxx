/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	setup.cxx - this file is integral part of `hector' project.

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

#include <cstdio>

#include <yaal/hcore/base.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include <yaal/tools/util.hxx>
#include <yaal/tools/hfsitem.hxx>

#include "setup.hxx"

using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector {

char const* const OSetup::DEFAULT_SOCKET_ROOT = "/tmp/hector/";

void OSetup::test_setup( void ) {
	M_PROLOG
	int failNo( 0 );
	++ failNo;
	if ( _quiet && _verbose )
		yaal::tools::util::failure( failNo,
				_( "quiet and verbose options are exclusive\n" ) );
	if ( _verbose )
		clog.reset( make_pointer<HFile>( stdout ) );
	else
		std::clog.rdbuf( NULL );
	if ( _quiet ) {
		cout.reset();
		std::cout.rdbuf( NULL );
	}
#if defined ( TARGET_HECTOR_DAEMON )
	++ failNo;
	if ( _dataDir.is_empty() )
		yaal::tools::util::failure( failNo,
				_( "you must specify directory with application data\n" ) );
	++ failNo;
	if ( _databaseName.is_empty() )
		yaal::tools::util::failure( failNo, _( "database name must be specified\n" ) );
	++ failNo;
	if ( _authQuery.is_empty() && ( _tableUser.is_empty() || _columnLogin.is_empty() || _columnPassword.is_empty() ) )
		yaal::tools::util::failure( failNo, _( "authentication query or table user with column login with column password must be specified\n" ) );

#endif
	++ failNo;
	if ( _maxConnections < 0 )
		yaal::tools::util::failure( failNo,
				_( "bad max_connection value set\n" ) );
	++ failNo;
	if ( _maxWorkingThreads < 0 )
		yaal::tools::util::failure( failNo,
				_( "bad max_working_threads value set\n" ) );
	++ failNo;
	if ( _socketWriteTimeout < 0 )
		yaal::tools::util::failure( failNo,
				_( "negative write timeout set\n" ) );
	HFSItem root( _socketRoot );
	++ failNo;
	if ( ! root.is_directory() )
		yaal::tools::util::failure( failNo,
				_( "socket root is invalid\n" ) );
#if defined ( TARGET_HECTOR_DAEMON )
	HFSItem data( _dataDir );
	++ failNo;
	if ( ! ( !! data && data.is_directory() ) ) {
		HString err( ! _dataDir.is_empty() ? _dataDir : "(nil)" );
		err += _( ": applications database path is invalid\n" );
		yaal::tools::util::failure( failNo, err.raw() );
	}
#elif defined ( TARGET_HECTOR_ADMIN )
	++ failNo;
	if ( ! ( _status || _shutdown || ! _reload.is_empty() || ! _restart.is_empty() ) )
		yaal::tools::util::failure( failNo, _( "you have to specify some administrative action\n" ) );
#endif
	return;
	M_EPILOG
}

}

