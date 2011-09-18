/*
---       `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski         ---

	options.cxx - this file is integral part of `hector' project.

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
#include <cstring>
#include <cstdio>

#include <yaal/hcore/base.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include <yaal/hcore/hprogramoptionshandler.hxx>
#include <yaal/tools/util.hxx>

#include "options.hxx"
#include "config.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector {

bool set_variables( HString& option_, HString& value_ ) {
	fprintf ( stdout, "option: [%s], value: [%s]\n",
			option_.raw(), value_.raw() );
	return ( false );
}

void version( void* ) __attribute__(( __noreturn__ ));
void version( void* ) {
	cout << PACKAGE_STRING << endl;
	throw ( 0 );
}

/* Set all the option flags according to the switches specified.
   Return the index of the first non-option argument.                    */
int handle_program_options( int argc_, char** argv_ ) {
	M_PROLOG
	HProgramOptionsHandler po;
	OOptionInfo info( po, setup._programName, "XML based Web Application Server.", NULL );
	bool stop = false;
	po( "log_path", program_options_helper::option_value( setup._logPath ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "path pointing to file for application logs", "path" )
#if defined ( TARGET_HECTOR_DAEMON )
		( "data_dir", program_options_helper::option_value( setup._dataDir ), "D", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "find application data here", "path" )
		( "database_name", program_options_helper::option_value( setup._databaseName ), 'N', HProgramOptionsHandler::OOption::TYPE::REQUIRED, "database connection path scheme", "path" )
		( "database_login", program_options_helper::option_value( setup._databaseLogin ), 'U', HProgramOptionsHandler::OOption::TYPE::REQUIRED, "datbase connection user name", "user" )
		( "database_password", program_options_helper::option_value( setup._databasePassword ), 'p', HProgramOptionsHandler::OOption::TYPE::REQUIRED, "database connection password", "password" )
		( "table_user", program_options_helper::option_value( setup._tableUser ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "datbase table with authentication data", "name" )
		( "column_login", program_options_helper::option_value( setup._columnLogin ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "columnt name with logins in authentication data table", "name" )
		( "column_password", program_options_helper::option_value( setup._columnPassword ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "columnt name with passwords in authentication data table", "name" )
		( "auth_query", program_options_helper::option_value( setup._authQuery ), HProgramOptionsHandler::OOption::TYPE::REQUIRED, "free form authentication query", "query" )
		( "max_connections", program_options_helper::option_value( setup._maxConnections ), "M", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "maximum number of concurent connections", "count" )
		( "max_working_threads", program_options_helper::option_value( setup._maxWorkingThreads ), "j", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "maximum number of internal task processing threads", "count" )
#elif defined ( TARGET_HECTOR_ADMIN )
		( "shutdown", program_options_helper::option_value( setup._shutdown ), "S", HProgramOptionsHandler::OOption::TYPE::NONE, "shutdown server nicely" )
		( "reload", program_options_helper::option_value( setup._reload ), "r", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "reload given application", "app" )
		( "restart", program_options_helper::option_value( setup._restart ), "A", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "restart given application (reboot activex)", "app" )
		( "status", program_options_helper::option_value( setup._status ), "i", HProgramOptionsHandler::OOption::TYPE::NONE, "print server information" )
#endif
		( "timeout_write", program_options_helper::option_value( setup._socketWriteTimeout ), "T", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "timeout for socket write operation", "seconds" )
		( "socket_root", program_options_helper::option_value( setup._socketRoot ), "R", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "root path for communication socket", "path" )
		( "quiet", program_options_helper::option_value( setup._quiet ), "q", HProgramOptionsHandler::OOption::TYPE::NONE, "inhibit usual output" )
		( "silent", program_options_helper::option_value( setup._quiet ), "q", HProgramOptionsHandler::OOption::TYPE::NONE, "inhibit usual output" )
		( "verbose", program_options_helper::option_value( setup._verbose ), "v", HProgramOptionsHandler::OOption::TYPE::NONE, "print more information" )
		( "help", program_options_helper::option_value( stop ), "h", HProgramOptionsHandler::OOption::TYPE::NONE, "display this help and stop", program_options_helper::callback( util::show_help, &info ) )
		( "dump-configuration", program_options_helper::option_value( stop ), "W", HProgramOptionsHandler::OOption::TYPE::NONE, "dump current configuration", program_options_helper::callback( util::dump_configuration, &info ) )
		( "version", program_options_helper::no_value, "V", HProgramOptionsHandler::OOption::TYPE::NONE, "output version information and stop", program_options_helper::callback( version, NULL ) );
	po.process_rc_file( "hector", "", NULL );
	if ( setup._logPath.is_empty() )
		setup._logPath = "hectord.log";
	int unknown = 0, nonOption = 0;
	nonOption = po.process_command_line( argc_, argv_, &unknown );
	if ( unknown > 0 ) {
		util::show_help( &info );
		throw unknown;
	}
	if ( stop )
		throw 0;
	return ( nonOption );
	M_EPILOG
}

}

