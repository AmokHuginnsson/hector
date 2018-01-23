/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <cstdlib>
#include <iostream>

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/signals.hxx>
#include <yaal/tools/util.hxx>
M_VCSID( "$Id: " __ID__ " $" )

#include "setup.hxx"
#include "options.hxx"
#include "applicationserver.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;
using namespace hector;

namespace hector {

OSetup setup;

}

int main( int argc_, char* argv_[] ) {
	HScopeExitCall sec( call( &HSignalService::stop, &HSignalService::get_instance() ) );
	M_PROLOG
	int ret( 0 );
	try {
/* TO-DO: enter main loop code here */
		HSignalService::get_instance();
		setup._programName = argv_[ 0 ];
		handle_program_options( argc_, argv_ );
		hcore::log.rehash( setup._logPath, setup._programName );
		setup.test_setup();
		HApplicationServer appServ;
		appServ.start();
/* *BOOM* */
		appServ.run();
		cerr << _( "Done" ) << endl;
/* ... there is the place main loop ends. :OD-OT */
	} catch ( int err ) {
		ret = err;
	}
	return ( ret );
	M_FINAL
}

