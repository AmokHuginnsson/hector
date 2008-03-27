/*
---       `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski         ---

	rc_options.cxx - this file is integral part of `hector' project.

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yaal/yaal.h>
M_VCSID ( "$Id$" )

#include "setup.h"

using namespace yaal;
using namespace yaal::hcore;

namespace hector
{

bool set_variables ( HString & a_roOption, HString & a_roValue )
	{
	fprintf ( stdout, "option: [%s], value: [%s]\n",
			static_cast < char const * const > ( a_roOption ),
			static_cast < char const * const > ( a_roValue ) );
	return ( false );
	}

int process_hectorrc_file( void )
	{
	OOption l_psVars[] =
		{
			{ "application", D_HSTRING, &setup.f_oApplication, "A", OOption::D_REQUIRED, "app", "name for served application", NULL },
			{ "data_dir", D_HSTRING, &setup.f_oDataDir, "D", OOption::D_REQUIRED, "path", "find application data here", NULL },
			{ "max_connections", D_INT, & setup.f_iMaxConnections, "M", OOption::D_REQUIRED, "count", "maximum number of concurent connections", NULL },
			{ "socket_root", D_HSTRING, &setup.f_oSocketRoot, "R", OOption::D_REQUIRED, "path", "root path for communication socket", NULL },
			{ "timeout_write", D_INT, & setup.f_iSocketWriteTimeout, "T", OOption::D_REQUIRED, "seconds", "timeout for socket write operation", NULL },
			{ "log_path", D_HSTRING, & setup.f_oLogPath, NULL, OOption::D_REQUIRED, "path", "path pointing to file for application logs", NULL },
			{ NULL, D_VOID, NULL, NULL, OOption::D_NONE, NULL, NULL, NULL }
		};
	rc_file::process_rc_file ( "hector", NULL, l_psVars, NULL );
	if ( ! setup.f_oLogPath )
		setup.f_oLogPath = "hectord.log";
	return ( 0 );
	}

}

