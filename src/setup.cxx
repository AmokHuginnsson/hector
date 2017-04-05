/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  setup.cxx - this file is integral part of `hector' project.

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

#include <cstdio>
#include <iostream>

#include "setup.hxx"
M_VCSID( "$Id: " __ID__ " $" )
#include <yaal/hcore/base.hxx>
#include <yaal/tools/util.hxx>
#include <yaal/tools/hfsitem.hxx>

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
		clog.reset( make_pointer<HFile>( stdout, HFile::OWNERSHIP::EXTERNAL ) );
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
	++ failNo;
	static int const MAX_PORT( 65535 );
	if ( ( _sslPort < 0 ) || ( _sslPort > MAX_PORT ) )
		yaal::tools::util::failure( failNo,
				_( "bad ssl port number\n" ) );
	HFSItem root( _socketRoot );
	++ failNo;
	if ( ! root.is_directory() )
		yaal::tools::util::failure( failNo,
				_( "socket root is invalid\n" ) );
#if defined ( TARGET_HECTOR_DAEMON )
	HFSItem data( _dataDir );
	++ failNo;
	if ( ! ( !! data && data.is_directory() ) )
		yaal::tools::util::failure( failNo, _( "%s: applications database path is invalid\n" ), ( ! _dataDir.is_empty() ? _dataDir.c_str() : "(nil)" ) );
#elif defined ( TARGET_HECTOR_ADMIN )
	++ failNo;
	if ( ! ( _status || _shutdown || ! _reload.is_empty() || ! _restart.is_empty() ) )
		yaal::tools::util::failure( failNo, _( "you have to specify some administrative action\n" ) );
#endif
	return;
	M_EPILOG
}

}

