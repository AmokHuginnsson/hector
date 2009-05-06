/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

	main.cxx - this file is integral part of `hector' project.

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

#include <unistd.h> /* the `char** environ;' variable */

#include <yaal/yaal.hxx> /* all hAPI headers */
M_VCSID ( "$Id$" )

#include "version.hxx"
#include "setup.hxx"
#include "options.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;
using namespace hector;

namespace hector
{

OSetup setup;

}

void query( void );

int main( int a_iArgc, char* a_ppcArgv[] )
	{
	M_PROLOG
/* variables declarations for main loop: */
	int l_iOpt = 0;
/* end. */
	try
		{
/* TO-DO: enter main loop code here */
		HSignalServiceFactory::get_instance();
		setup.f_pcProgramName = a_ppcArgv[ 0 ];
		l_iOpt = handle_program_options( a_iArgc, a_ppcArgv );
		setup.f_oLogPath.replace( "hectord", "hectoradmin" );
		hcore::log.rehash( setup.f_oLogPath, setup.f_pcProgramName );
		setup.test_setup();
/*		if ( ! cons.is_enabled() )
			enter_curses(); */ /* enabling ncurses ablilities */
/* *BOOM* */
		query();
/* ... there is the place main loop ends. :OD-OT */
		}
	catch ( ... )
		{
		throw;
		}
	return ( 0 );
	M_FINAL
	}

void show_answer( HSocket& sock )
	{
	M_PROLOG
	HString msg;
	while ( sock.read_until( msg ).octets >= 0 )
		cout << msg << endl;
	return;
	M_EPILOG
	}

void query_status( void )
	{
	M_PROLOG
	HString sockPath( setup.f_oSocketRoot );
	sockPath += "/control.sock";
	try
		{
		HSocket sock( HSocket::TYPE::FILE );
		sock.connect( sockPath );
		sock << "status" << endl;
		show_answer( sock );
		}
	catch ( HSocketException& e )
		{
		cout << "Cannot connect to `hector' daemon." << endl;
		cout << e.what() << endl;
		}
	return;
	M_EPILOG
	}

void query_shutdown( void )
	{
	M_PROLOG
	HString sockPath( setup.f_oSocketRoot );
	sockPath += "/control.sock";
	try
		{
		HSocket sock( HSocket::TYPE::FILE );
		sock.connect( sockPath );
		sock << "shutdown" << endl;
		show_answer( sock );
		}
	catch ( HSocketException& e )
		{
		cout << "Cannot connect to `hector' daemon." << endl;
		cout << e.what() << endl;
		}
	return;
	M_EPILOG
	}

void query_reload( void )
	{
	M_PROLOG
	HString sockPath( setup.f_oSocketRoot );
	sockPath += "/control.sock";
	try
		{
		HSocket sock( HSocket::TYPE::FILE );
		sock.connect( sockPath );
		sock << "reload:" << setup.f_oReload << endl;
		show_answer( sock );
		}
	catch ( HSocketException& e )
		{
		cout << "Cannot connect to `hector' daemon." << endl;
		cout << e.what() << endl;
		}
	return;
	M_EPILOG
	}

void query( void )
	{
	M_PROLOG
	if ( ! setup.f_oReload.is_empty() )
		query_reload();
	if ( setup.f_bStatus )
		query_status();
	if ( setup.f_bShutdown )
		query_shutdown();
	return;
	M_EPILOG
	}

