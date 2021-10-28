/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h> /* the `char** environ;' variable */

#include <yaal/hcore/hstring.hxx> /* all hAPI headers */
M_VCSID ( "$Id$" )
#include <yaal/hcore/hsocket.hxx>
#include <yaal/hcore/hlog.hxx>
#include <yaal/hcore/htokenizer.hxx>
#include <yaal/tools/hthreadpool.hxx>
#include <yaal/tools/signals.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/tools/util.hxx>

#include "setup.hxx"
#include "options.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;
using namespace hector;

namespace hector {

OSetup setup;

}

void query( int, char** );

int main( int argc_, char* argv_[] ) {
	HScopeExitCall secTP( call( &HThreadPool::stop, &HThreadPool::get_instance() ) );
	HScopeExitCall sec( call( &HSignalService::stop, &HSignalService::get_instance() ) );
	M_PROLOG
	int ret( 0 );
	try {
/* TO-DO: enter main loop code here */
		HSignalService::get_instance();
		setup._programName = argv_[ 0 ];
		handle_program_options( argc_, argv_ );
		setup._logPath.replace( "hectord", "hector.cgi" );
		hcore::log.rehash( setup._logPath, setup._programName );
		setup.test_setup();
/* *BOOM* */
		query( argc_, argv_ );
/* ... there is the place main loop ends. :OD-OT */
	} catch ( int err ) {
		ret = err;
	} catch ( ... ) {
		throw;
	}
	return ret;
	M_FINAL
}

namespace {

HString escape( HString const& source ) {
	M_PROLOG
	static HString result;
	result = source;
	result.trim_left().replace( "\\", "\\\\" ).replace( "\n", "\\n" );
	return result;
	M_EPILOG
}

void push_query( HSocket& sock, HString const& query, char const* const mode, char const* const delim ) {
	M_PROLOG
	HStringStream buffer;
	HTokenizer t( query, delim, HTokenizer::SKIP_EMPTY );
	for ( HTokenizer::HIterator it = t.begin(), end = t.end(); it != end; ++ it ) {
		sock << ( buffer << mode << ":" << escape( *it ) << endl << buffer );
	}
	return;
	M_EPILOG
}

}

void query( int argc, char** argv ) {
	M_PROLOG
	HString sockPath( setup._socketRoot );
	sockPath += "/request.sock";
	try {
		char const* REQUEST_METHOD = getenv( "REQUEST_METHOD" );
		HSocket sock( HSocket::TYPE::FILE );
		sock.connect( sockPath );
		if ( ! REQUEST_METHOD || ( stricasecmp( REQUEST_METHOD, "POST" ) == 0 ) ) {
			HFile in( stdin, HFile::OWNERSHIP::EXTERNAL );
			HString POST( "" );
			in.set_buffered_io( false );
			while ( getline( in, POST ).good() ) {
				push_query( sock, POST, "post", "&" );
			}
		}
		HStringStream buffer;
		for ( int i = 1; i < argc; ++ i ) {
			sock << ( buffer << "get:" << escape( argv[ i ] ) << endl << buffer );
		}
		char QS[] = "QUERY_STRING=";
		char CS[] = "HTTP_COOKIE=";
		for ( int i = 0; environ[ i ]; ++ i ) {
			if ( ! strncmp( environ[ i ], QS, sizeof ( QS ) - 1 ) ) {
				push_query( sock, environ[ i ] + sizeof ( QS ) - 1, "get", "&" );
				continue;
			} else if ( ! strncmp( environ[ i ], CS, sizeof ( CS ) - 1 ) ) {
				push_query( sock, environ[ i ] + sizeof ( CS ) - 1, "cookie", ";" );
				continue;
			}
			buffer << "env:" << escape( environ[ i ] ) << endl;
			sock << buffer.consume();
		}
		sock << "done" << endl;
		HString msg;
		while ( sock.read_until( msg ) > 0 ) {
			cout << msg;
		}
		cout.flush();
	} catch ( HSocketException& e ) {
		cout << "\n\nCannot connect to `hector' daemon.<br />" << endl;
		cout << e.what() << endl;
	}
	return;
	M_EPILOG
}

