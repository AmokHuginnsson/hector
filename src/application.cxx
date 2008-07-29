/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	application.cxx - this file is integral part of `hector' project.

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

#include <iostream>

#include <yaal/yaal.h>
M_VCSID( "$Id: "__ID__" $" )
#include "application.h"
#include "applicationserver.h"
#include "setup.h"
#include "cgi.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::hconsole;
using namespace yaal::tools;
using namespace yaal::tools::util;

namespace hector
{

HApplication::HApplication( void )
	: f_oDOM(), f_oName()
	{
	}

HApplication::~HApplication( void )
	{
	out << "Application `" << f_oName << "' unloaded." << endl;
	}

OProcessor OProcessor::get_instance( HString const& name, HString const& path )
	{
	M_PROLOG
	static char const* const D_SYMBOL_FACTORY = "factory";
	static char const* const D_ATTRIBUTE_ACTIVEX = "activex";
	HStringStream activex( path );
	HPlugin::ptr_t l_oActiveX( new HPlugin() );
	activex << "/" << name << "/" << D_ATTRIBUTE_ACTIVEX;
	HApplication::ptr_t app;
	l_oActiveX->load( activex.raw() );
	M_ASSERT( l_oActiveX->is_loaded() );
	out << "activex nest for `" << name << "' loaded" << endl;
	typedef HApplication::ptr_t ( *factory_t )( void );
	factory_t factory;
	l_oActiveX->resolve( D_SYMBOL_FACTORY, factory );
	M_ASSERT( factory );
	out << "activex factory for `" << name << "' connected" << endl;
	app = factory();
	if ( ! app )
		throw HApplicationException( "invalid activex" );
	OProcessor proc( activex.raw() );
	proc.f_oApplication = app;
	proc.f_oActiveX = l_oActiveX;
	app->load( name, path );
	return ( proc );
	M_EPILOG
	}

void OProcessor::reload_binary( void )
	{
	M_PROLOG
	f_oActiveX->unload();
	f_oActiveX->load( f_oBinaryPath );
	return;
	M_EPILOG
	}

void HApplication::load( HString const& name, HString const& path )
	{
	M_PROLOG
	static char const* const D_INTERFACE_FILE = "interface.xml";
	static char const* const D_TOOLKIT_FILE = "toolkit.xml";
	f_oName = name;
	HStringStream interface( path );
	HStringStream toolkit( path );
	hcore::log( LOG_TYPE::D_INFO ) << "Loading application `" << f_oName << "'." << endl;
	interface << "/" << f_oName << "/" << D_INTERFACE_FILE;
	toolkit << "/" << f_oName << "/" << D_TOOLKIT_FILE;
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << interface.raw() << "' as application template." << endl;
	f_oDOM.init( interface.raw() );
	hcore::log( LOG_TYPE::D_INFO ) << "Using `" << toolkit.raw() << "' as a toolkit library." << endl;
	f_oDOM.apply_style( toolkit.raw() );
	f_oDOM.parse();
	do_load();
	return;	
	M_EPILOG
	}

void HApplication::do_load( void )
	{
	out << __PRETTY_FUNCTION__ << endl;
	}

void HApplication::do_handle_logic( ORequest& req )
	{
	M_PROLOG
	out << __PRETTY_FUNCTION__ << endl;
	static char const D_LOGIC_PATH[] = "/html/logic/";
	HXml::HNodeProxy logic = f_oDOM.get_element_by_path( D_LOGIC_PATH );
	if ( !! logic )
		cgi::make_cookies( logic, req );
	return;
	M_EPILOG
	}

void HApplication::do_generate_page( ORequest const& req )
	{
	M_PROLOG
	out << __PRETTY_FUNCTION__ << endl;
	cgi::default_t defaults;
	cgi::waste_children( dom().get_root(), req, defaults );
	cgi::mark_children( dom().get_root(), req, defaults, dom() );
	cgi::move_children( dom().get_root(), req, dom() );
	cgi::expand_autobutton( dom().get_root(), req );
	return;
	M_EPILOG
	}

void HApplication::generate_page( ORequest const& req )
	{
	M_PROLOG
	out << __PRETTY_FUNCTION__ << endl;
	do_generate_page( req );
	f_oDOM.save( req.socket()->get_file_descriptor() );
	return;
	M_EPILOG
	}

void HApplication::handle_logic( ORequest& req )
	{
	M_PROLOG
	out << __PRETTY_FUNCTION__ << endl;
	do_handle_logic( req );
	return;
	M_EPILOG
	}

HXml& HApplication::dom( void )
	{
	return ( f_oDOM );
	}

}

