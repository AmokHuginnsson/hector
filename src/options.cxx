/*
---       `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski         ---

	options.cxx - this file is integral part of `hector' project.

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
#include <cstring>
#include <cstdio>

#include <yaal/hcore/hprogramoptionshandler.hxx>
#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/util.hxx>
M_VCSID( "$Id: " __ID__ " $" )

#include "options.hxx"
#include "config.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector {

namespace {

bool set_variables( HString& option_, HString& value_ ) {
	static bool const HECTOR_RC_DEBUG( !! ::getenv( "HECTOR_RC_DEBUG" ) );
	if ( HECTOR_RC_DEBUG ) {
		cout << "option: [" << option_ << "], value: [" << value_ << "]" << endl;
	}
	return ( true );
}

void version( void* ) {
	cout << PACKAGE_STRING << endl;
	return;
}

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
		( "ssl_port", program_options_helper::option_value( setup._sslPort ), "S", HProgramOptionsHandler::OOption::TYPE::REQUIRED, "port number for SSL based connections", "number" )
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
		( "version", program_options_helper::option_value( stop ), 'V', HProgramOptionsHandler::OOption::TYPE::NONE, "output version information and stop", program_options_helper::callback( version, NULL ) );
	po.process_rc_file( "hector", "", set_variables );
	if ( setup._logPath.is_empty() )
		setup._logPath = "hectord.log";
	int unknown( 0 );
	int nonOption( po.process_command_line( argc_, argv_, &unknown ) );
	if ( stop || ( unknown > 0 ) ) {
		if ( unknown > 0 )
			util::show_help( &info );
		HLog::disable_auto_rehash();
		throw unknown;
	}
	return ( nonOption );
	M_EPILOG
}

}

