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

#include "config.hxx"

#include <yaal/hcore/hprogramoptionshandler.hxx>
#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/util.hxx>
M_VCSID( "$Id: " __ID__ " $" )

#include "options.hxx"
#include "setup.hxx"
#include "commit_id.hxx"

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

void version( void ) {
	cout << PACKAGE_STRING << ( setup._verbose ? " " COMMIT_ID : "" ) << endl;
	return;
}

}

/* Set all the option flags according to the switches specified.
   Return the index of the first non-option argument.                    */
int handle_program_options( int argc_, char** argv_ ) {
	M_PROLOG
	HProgramOptionsHandler po;
	OOptionInfo info( po, setup._programName, "XML based Web Application Server.", NULL );
	bool help( false );
	bool conf( false );
	bool vers( false );
	po(
		HProgramOptionsHandler::HOption()
		.long_form( "log_path" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "path pointing to file for application logs" )
		.recipient(	setup._logPath )
		.argument_name( "path" )
		.default_value( "hectord.log" )
#if defined ( TARGET_HECTOR_DAEMON )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'D' )
		.long_form( "data_dir" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "find application data here" )
		.recipient( setup._dataDir )
		.argument_name( "path" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'N' )
		.long_form( "database_name" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "database connection path scheme" )
		.recipient( setup._databaseName )
		.argument_name( "path" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'U' )
		.long_form( "database_login" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "datbase connection user name" )
		.recipient( setup._databaseLogin )
		.argument_name( "user" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'p' )
		.long_form( "database_password" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "database connection password" )
		.recipient( setup._databasePassword )
		.argument_name( "password" )
	)(
		HProgramOptionsHandler::HOption()
		.long_form( "table_user" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "datbase table with authentication data" )
		.recipient( setup._tableUser )
		.argument_name( "name" )
	)(
		HProgramOptionsHandler::HOption()
		.long_form( "column_login" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "columnt name with logins in authentication data table" )
		.recipient( setup._columnLogin )
		.argument_name( "name" )
	)(
		HProgramOptionsHandler::HOption()
		.long_form( "column_password" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "columnt name with passwords in authentication data table" )
		.recipient( setup._columnPassword )
		.argument_name( "name" )
	)(
		HProgramOptionsHandler::HOption()
		.long_form( "auth_query" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "free form authentication query" )
		.recipient( setup._authQuery )
		.argument_name( "query" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'M' )
		.long_form( "max_connections" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "maximum number of concurent connections" )
		.recipient( setup._maxConnections )
		.argument_name( "count" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'j' )
		.long_form( "max_working_threads" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "maximum number of internal task processing threads" )
		.recipient( setup._maxWorkingThreads )
		.argument_name( "count" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'S' )
		.long_form( "ssl_port" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "port number for SSL based connections" )
		.recipient( setup._sslPort )
		.argument_name( "number" )
#elif defined ( TARGET_HECTOR_ADMIN )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'S' )
		.long_form( "shutdown" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "shutdown server nicely" )
		.recipient( setup._shutdown )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'r' )
		.long_form( "reload" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "reload given application" )
		.recipient( setup._reload )
		.argument_name( "app" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'A' )
		.long_form( "restart" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "restart given application (reboot activex)" )
		.recipient( setup._restart )
		.argument_name( "app" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'i' )
		.long_form( "status" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "print server information" )
		.recipient( setup._status )
#endif
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'T' )
		.long_form( "timeout_write" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "timeout for socket write operation" )
		.recipient( setup._socketWriteTimeout )
		.argument_name( "seconds" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'R' )
		.long_form( "socket_root" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::REQUIRED )
		.description( "root path for communication socket" )
		.recipient( setup._socketRoot )
		.argument_name( "path" )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'q' )
		.long_form( "quiet" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "inhibit usual output" )
		.recipient( setup._quiet )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'q' )
		.long_form( "silent" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "inhibit usual output" )
		.recipient( setup._quiet )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'v' )
		.long_form( "verbose" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "print more information" )
		.recipient( setup._verbose )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'h' )
		.long_form( "help" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "display this help and stop" )
		.recipient( help )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'W' )
		.long_form( "dump-configuration" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "dump current configuration" )
		.recipient( conf )
	)(
		HProgramOptionsHandler::HOption()
		.short_form( 'V' )
		.long_form( "version" )
		.switch_type( HProgramOptionsHandler::HOption::ARGUMENT::NONE )
		.description( "output version information and stop" )
		.recipient( vers )
	);
	po.process_rc_file( "hector", "", set_variables );
	int unknown( 0 );
	int nonOption( po.process_command_line( argc_, argv_, &unknown ) );
	if ( help || conf || vers || ( unknown > 0 ) ) {
		if ( help || ( unknown > 0 ) ) {
			util::show_help( info );
		} else if ( conf ) {
			util::dump_configuration( info );
		} else if ( vers ) {
			version();
		}
		HLog::disable_auto_rehash();
		throw unknown;
	}
	return ( nonOption );
	M_EPILOG
}

}

