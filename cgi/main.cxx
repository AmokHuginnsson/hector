/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

	main.cxx - this file is integral part of `hector' project.

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

#include <iostream>

#include <yaal/yaal.h> /* all hAPI headers */
M_VCSID ( "$Id$" )

#include "version.h"
#include "setup.h"
#include "cli_options.h"
#include "rc_options.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

OSetup setup;

int main( int a_iArgc, char* a_ppcArgv[] )
	{
	M_PROLOG
/* variables declarations for main loop: */
	int l_iOpt = 0;
	HConsole& cons = HCons::get_instance();
/* end. */
	try
		{
/* TO-DO: enter main loop code here */
		HSignalServiceFactory::get_instance();
		setup.f_pcProgramName = a_ppcArgv[ 0 ];
		process_hectorrc_file();
		l_iOpt = decode_switches( a_iArgc, a_ppcArgv );
		hcore::log.rehash( setup.f_oLogPath, setup.f_pcProgramName );
		setup.test_setup();
/*		if ( ! cons.is_enabled() )
			enter_curses(); */ /* enabling ncurses ablilities */
/* *BOOM* */
		if ( cons.is_enabled() )
			cons.leave_curses(); /* ending ncurses sesion */
/* ... there is the place main loop ends. :OD-OT */
		}
	catch ( ... )
		{
		if ( cons.is_enabled() )
			cons.leave_curses(); /* ending ncurses sesion */
		throw;
		}
	::fprintf( stderr, "Done.\n" );
	return ( 0 );
	M_FINAL
	}

