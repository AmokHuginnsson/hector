/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

  main.cxx - this file is integral part of `hector' project.

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

#include <cstdlib>
#include <unistd.h> /* the `char** environ;' variable */

#include <yaal/hcore/hsocket.hxx>
#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/signals.hxx>
#include <yaal/tools/util.hxx>
M_VCSID ( "$Id$" )

#include "setup.hxx"
#include "options.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;
using namespace hector;

namespace hector {

OSetup setup;

}

void query( void );

int main( int argc_, char* argv_[] ) {
	HScopeExitCall sec( call( &HSignalService::stop, &HSignalService::get_instance() ) );
	M_PROLOG
	int ret( 0 );
	try {
/* TO-DO: enter main loop code here */
		HSignalService::get_instance();
		setup._programName = argv_[ 0 ];
		handle_program_options( argc_, argv_ );
		setup._logPath.replace( "hectord", "hectoradmin" );
		hcore::log.rehash( setup._logPath, setup._programName );
		setup.test_setup();
/* *BOOM* */
		query();
/* ... there is the place main loop ends. :OD-OT */
	} catch ( int err ) {
		ret = err;
	}
	return ( ret );
	M_FINAL
}

namespace {

void show_answer( HSocket& sock ) {
	M_PROLOG
	HString msg;
	int long nRead( 0 );
	bool err( false );
	while ( ( nRead = sock.read_until( msg ) ) ) {
		if ( nRead < 0 ) {
			err = true;
			continue;
		}
		cout << msg << endl;
	}
	cout << "status: " << ( ! err ? "ok" : "error" ) << endl;
	return;
	M_EPILOG
}

void query_status( void ) {
	M_PROLOG
	HString sockPath( setup._socketRoot );
	sockPath += "/control.sock";
	HSocket sock( HSocket::TYPE::FILE );
	sock.connect( sockPath );
	sock << "status" << endl;
	show_answer( sock );
	return;
	M_EPILOG
}

void query_shutdown( void ) {
	M_PROLOG
	HString sockPath( setup._socketRoot );
	sockPath += "/control.sock";
	HSocket sock( HSocket::TYPE::FILE );
	sock.connect( sockPath );
	sock << "shutdown" << endl;
	show_answer( sock );
	return;
	M_EPILOG
}

void query_restart_reload( char const* const action_, HString const& object_ ) {
	M_PROLOG
	HString sockPath( setup._socketRoot );
	sockPath += "/control.sock";
	HSocket sock( HSocket::TYPE::FILE );
	sock.connect( sockPath );
	sock << action_ << object_ << endl;
	show_answer( sock );
	return;
	M_EPILOG
}

}

void query( void ) {
	M_PROLOG
	try {
		if ( ! setup._reload.is_empty() )
			query_restart_reload( "reload:", setup._reload );
		if ( ! setup._restart.is_empty() )
			query_restart_reload( "restart:", setup._restart );
		if ( setup._status )
			query_status();
		if ( setup._shutdown )
			query_shutdown();
	} catch ( HSocketException& e ) {
		cout << "Cannot connect to `hector' daemon." << endl;
		cout << e.what() << endl;
	}
	return;
	M_EPILOG
}

