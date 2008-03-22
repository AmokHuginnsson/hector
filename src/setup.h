/*
---            `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski             ---

	setup.h - this file is integral part of `hector' project.

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

#ifndef __SETUP_H
#define __SETUP_H

#include <libintl.h>
#include <yaal/yaal.h>

#define out ( cout << __FILE__ + OSetup::PATH_OFFSET << ":" << __LINE__ << ": " )

struct OSetup
	{
	static int const D_DEFAULT_MAX_CONNECTIONS = 2;
	static char const* const D_SOCK_ROOT;
	bool f_bQuiet;			/* --quiet, --silent */
	bool f_bVerbose;		/* --verbose */
	bool f_bHelp;
	char * f_pcProgramName;
	int f_iMaxConnections;
	yaal::hcore::HString f_oLogPath;
	yaal::hcore::HString f_oApplication;
	yaal::hcore::HString f_oDataDir;
	/* self-sufficient */
	static int const PATH_OFFSET = sizeof ( __FILE__ ) - sizeof ( "setup.h" );
	OSetup( void ) : f_bQuiet( false ), f_bVerbose( false ),
										f_bHelp( false ), f_pcProgramName( NULL ),
										f_iMaxConnections( D_DEFAULT_MAX_CONNECTIONS ),
										f_oLogPath(), f_oApplication(), f_oDataDir() {}
	void test_setup( void );
private:
	OSetup ( OSetup const & );
	OSetup & operator = ( OSetup const & );
	};

extern OSetup setup;

#endif /* __SETUP_H */
