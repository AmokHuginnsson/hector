/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	oactivex.cxx - this file is integral part of `hector' project.

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
#include "hactivex.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::tools::util;
using namespace yaal::dbwrapper;

namespace hector
{

HActiveX HActiveX::get_instance( HString const& name, HString const& path, HDataBase::ptr_t db_ )
	{
	M_PROLOG
	static char const* const SYMBOL_FACTORY = "factory";
	static char const* const ATTRIBUTE_ACTIVEX = "activex";
	HStringStream activex( path );
	HPlugin::ptr_t activeX( make_pointer<HPlugin>() );
	activex << "/" << name << "/" << ATTRIBUTE_ACTIVEX;
	HApplication::ptr_t app;
	out << "Trying path: `" << activex.raw() << "' for activex: `" << name << "'" << endl;
	activeX->load( activex.raw() );
	M_ASSERT( activeX->is_loaded() );
	out << "activex nest for `" << name << "' loaded" << endl;
	typedef HApplication::ptr_t ( *factory_t )( HDataBase::ptr_t );
	factory_t factory;
	activeX->resolve( SYMBOL_FACTORY, factory );
	M_ASSERT( factory );
	out << "activex factory for `" << name << "' connected" << endl;
	app = factory( db_ );
	if ( ! app )
		throw HApplicationException( "invalid activex" );
	HActiveX proc( activex.raw() );
	proc._application = app;
	proc._activeX = activeX;
	app->load( name, path );
	return ( proc );
	M_EPILOG
	}

void HActiveX::reload_binary( void )
	{
	M_PROLOG
	_activeX->unload();
	_activeX->load( _binaryPath );
	return;
	M_EPILOG
	}

void HActiveX::handle_logic( ORequest& req_, HSession& session_ )
	{
	M_PROLOG
	_application->handle_logic( req_, session_ );
	return;
	M_EPILOG
	}

void HActiveX::generate_page( ORequest const& req_, HSession const& session_ )
	{
	M_PROLOG
	_application->generate_page( req_, session_ );
	return;
	M_EPILOG
	}

HApplication::sessions_t& HActiveX::sessions( void )
	{
	return ( _application->sessions() );
	}

HApplication::sessions_t const& HActiveX::sessions( void ) const
	{
	return ( _application->sessions() );
	}

}

