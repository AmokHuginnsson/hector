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

#define out ( cout << __FILE__ + OSetup::PATH_OFFSET << ":" << __LINE__ << ": " )

namespace hector
{

inline std::ostream& operator << ( std::ostream& o, yaal::hcore::HString const& s )
	{ return ( o << s.raw() ); }

struct OSetup
	{
	static int const DEFAULT_MAX_CONNECTIONS = 2;
	static int const DEFAULT_SOCKET_WRITE_TIMEOUT = 5; /* seconds */
	static char const* const DEFAULT_SOCKET_ROOT;
	bool f_bQuiet;			/* --quiet, --silent */
	bool f_bVerbose;		/* --verbose */
	bool f_bShutdown;
	bool f_bStatus;
	char* f_pcProgramName;
	int f_iMaxConnections;
	int f_iSocketWriteTimeout;
	yaal::hcore::HString f_oLogPath;
	yaal::hcore::HString f_oDataDir;
	yaal::hcore::HString f_oSocketRoot;
	yaal::hcore::HString f_oReload;
	/* self-sufficient */
	static int const PATH_OFFSET = sizeof ( __FILE__ ) - sizeof ( "setup.hxx" );
	OSetup( void )
		: f_bQuiet( false ), f_bVerbose( false ),
		f_bShutdown( false ),
		f_bStatus( false ), f_pcProgramName( NULL ),
		f_iMaxConnections( DEFAULT_MAX_CONNECTIONS ),
		f_iSocketWriteTimeout( DEFAULT_SOCKET_WRITE_TIMEOUT ),
		f_oLogPath(), f_oDataDir(), f_oSocketRoot( DEFAULT_SOCKET_ROOT ),
		f_oReload() {}
	void test_setup( void );
private:
	OSetup ( OSetup const & );
	OSetup & operator = ( OSetup const & );
	};

extern OSetup setup;

}

#endif /* SETUP_HXX_INCLUDED */
