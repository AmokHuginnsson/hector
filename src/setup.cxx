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

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "setup.hxx"

using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector
{

char const* const OSetup::D_DEFAULT_SOCKET_ROOT = "/tmp/hector/";

void OSetup::test_setup( void )
	{
	M_PROLOG
	if ( f_bQuiet && f_bVerbose )
		yaal::tools::util::failure( 1,
				_( "quiet and verbose options are exclusive\n" ) );
#if defined ( TARGET_HECTOR_DAEMON )
	if ( f_oDataDir.is_empty() )
		yaal::tools::util::failure( 2,
				_( "you must specify directory with application data\n" ) );
#endif
	if ( f_iMaxConnections < 0 )
		yaal::tools::util::failure( 3,
				_( "bad max-connection value set\n" ) );
	if ( f_iSocketWriteTimeout < 0 )
		yaal::tools::util::failure( 4,
				_( "negative write timeout set\n" ) );
	HFSItem root( f_oSocketRoot );
	if ( ! root.is_directory() )
		yaal::tools::util::failure( 5,
				_( "socket root is invalid\n" ) );
#if defined ( TARGET_HECTOR_DAEMON )
	HFSItem data( f_oDataDir );
	if ( ! ( !! data && data.is_directory() ) )
		{
		HString err( ! f_oDataDir.is_empty() ? f_oDataDir : "(nil)" );
		err += _( ": applications database path is invalid\n" );
		yaal::tools::util::failure( 6, err.raw() );
		}
#elif defined ( TARGET_HECTOR_ADMIN )
	if ( ! ( f_bStatus || f_bShutdown || ! f_oReload.is_empty() ) )
		yaal::tools::util::failure( 7, _( "you have to specify some administrative action\n" ) );
#endif
	return;
	M_EPILOG
	}

}

