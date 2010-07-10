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

#include <cstdlib>
#include <iostream>

#include <yaal/yaal.hxx> /* all hAPI headers */
M_VCSID( "$Id: "__ID__" $" )

#include "setup.hxx"
#include "options.hxx"
#include "applicationserver.hxx"

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

int main( int argc_, char* argv_[] )
	{
	M_AT_END_OF_SCOPE( HSignalService::get_instance().stop(); );
	M_PROLOG
/* variables declarations for main loop: */
	int opt = 0;
	HConsole& cons = HConsole::get_instance();
/* end. */
	try
		{
/* TO-DO: enter main loop code here */
		HSignalService::get_instance();
		setup._programName = argv_[ 0 ];
		opt = handle_program_options( argc_, argv_ );
		hcore::log.rehash( setup._logPath, setup._programName );
		setup.test_setup();
		HApplicationServer appServ;
		appServ.start();
		appServ.run();
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
	cerr << _( "Done" ) << endl;
	return ( 0 );
	M_FINAL
	}

