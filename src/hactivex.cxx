/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <yaal/hcore/hfile.hxx>
#include <yaal/tools/hstringstream.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "hactivex.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

HActiveX HActiveX::get_instance(
	HApplicationServer* applicationServer_,
	HString const& id_,
	HString const& code_,
	HString const& name_,
	HString const& path_,
	HApplication::MODE mode_
) {
	M_PROLOG
	static char const* const SYMBOL_FACTORY = "factory";
	static char const* const ATTRIBUTE_ACTIVEX = "activex";
	HStringStream activex( path_ );
	HPlugin::ptr_t activeX( make_pointer<HPlugin>() );
	activex << "/" << code_ << "/" << ATTRIBUTE_ACTIVEX;
	HApplication::ptr_t app;
	OUT << "Trying path: `" << activex.str() << "' for activex: `" << id_ << "'" << endl;
	activeX->load( activex.str() );
	M_ASSERT( activeX->is_loaded() );
	OUT << "activex nest for `" << id_ << "' loaded" << endl;
	typedef HApplication::ptr_t ( *factory_t )( void );
	factory_t factory;
	activeX->resolve( SYMBOL_FACTORY, factory );
	M_ASSERT( factory );
	OUT << "activex factory for `" << id_ << "' connected" << endl;
	app = factory();
	if ( ! app ) {
		throw HActiveXException( "invalid activex" );
	}
	HActiveX proc( activex.str() );
	proc._application = app;
	proc._activeX = activeX;
	app->init();
	app->load( applicationServer_, id_, code_, name_, path_, mode_ );
	return proc;
	M_EPILOG
}

void HActiveX::reload_binary( void ) {
	M_PROLOG
	_activeX->unload();
	_activeX->load( _binaryPath );
	return;
	M_EPILOG
}

void HActiveX::handle_logic( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	_application->handle_logic( req_, session_ );
	return;
	M_EPILOG
}

void HActiveX::generate_page( ORequest const& req_, HSession const& session_ ) {
	M_PROLOG
	_application->generate_page( req_, session_ );
	return;
	M_EPILOG
}

HApplication::sessions_t& HActiveX::sessions( void ) {
	return ( _application->sessions() );
}

HApplication const& HActiveX::app( void ) const {
	return ( *_application );
}

}

