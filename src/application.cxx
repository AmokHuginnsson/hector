/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	application.cxx - this file is integral part of `hector' project.

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

#include <yaal/hcore/hlog.hxx>
#include <yaal/hcore/hformat.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/tools/hfsitem.hxx>
#include <yaal/tools/hash.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "application.hxx"
#include "applicationserver.hxx"
#include "setup.hxx"
#include "cgi.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

namespace {
static char const HACTION[] = "h-action";
}

HApplication::HApplication( void )
	: _dom()
	, _id()
	, _code()
	, _name()
	, _dsn()
	, _tableUser()
	, _columnLogin()
	, _columnPassword()
	, _authQuery()
	, _defaultSecurityContext()
	, _sessions()
	, _db()
	, _forms()
	, _mode( MODE::GET )
	, _hash( HASH::SHA1 ) {
	return;
}

HApplication::~HApplication( void ) {
	OUT << "Application `" << _name << "' unloaded." << endl;
}

void HApplication::init( void ) {
	M_PROLOG
	do_init();
	return;
	M_EPILOG
}

void HApplication::load(
	HApplicationServer* applicationServer_,
	HString const& id_,
	HString const& code_,
	HString const& name_,
	HString const& path_,
	MODE mode_
) {
	M_PROLOG
	static char const* const INTERFACE_FILE = "interface.xml";
	static char const* const TOOLKIT_FILE = "toolkit.xml";
	_id = id_;
	_code = code_;
	_name = name_;
	_mode = mode_;
	HStringStream interface( path_ );
	HStringStream toolkit( path_ );
	hcore::log( LOG_LEVEL::INFO ) << "Loading application `" << _id << "'." << endl;
	interface << "/" << _code << "/" << INTERFACE_FILE;
	toolkit << "/" << _code << "/" << TOOLKIT_FILE;
	HFSItem app( interface.string() );
	hcore::log( LOG_LEVEL::INFO ) << "Using `" << interface.string() << "' as application template." << endl;
	_dom.init( make_pointer<HFile>( interface.string(), HFile::OPEN::READING ), HXml::PARSER::RESOLVE_ENTITIES | HXml::PARSER::AUTO_XINCLUDE );
	hcore::log( LOG_LEVEL::INFO ) << "Using `" << toolkit.string() << "' as a toolkit library." << endl;
	_dom.parse( HXml::PARSER::STRIP_COMMENT | HXml::PARSER::IGNORE_CONVERSION_ERRORS );
	HXml::entities_t const& entities( _dom.entities() );
	_dsn = entities.at( "DSN" );
	HXml::entities_t::const_iterator aq( entities.find( "authQuery" ) );
	if ( aq != entities.end() ) {
		_authQuery = aq->second;
	} else {
		_tableUser = entities.at( "usersTable" );
		_columnLogin = entities.at( "userNameColumn" );
		_columnPassword = entities.at( "userPasswordColumn" );
	}
	HString h( entities.at( "hash" ) );
	if ( h.lower() == "sha1" ) {
		_hash = HASH::SHA1;
	} else if ( h == "md5" ) {
		_hash = HASH::MD5;
	} else {
		throw HApplicationException( "Bad hash type: "_ys.append( h ) );
	}
	_db = applicationServer_->get_db_connection( _dsn );
	HXml::entities_t::const_iterator u( entities.find( "user" ) );
	_defaultSecurityContext._user = u != entities.end() ? u->second : app.get_user();
	HXml::entities_t::const_iterator g( entities.find( "group" ) );
	_defaultSecurityContext._group = g != entities.end() ? g->second : app.get_group();
	HXml::entities_t::const_iterator m( entities.find( "permissions" ) );
	_defaultSecurityContext._mode = static_cast<ACCESS::mode_t>( m != entities.end() ? lexical_cast<int>( m->second ) : app.get_permissions() );
	cgi::prepare_logic( this, _dom.get_root() );
	_dom.apply_style( toolkit.string(), {{ "mode", _mode == MODE::GET ? "'GET'" : "'POST'" }} );
	_dom.parse( HXml::PARSER::STRIP_COMMENT | HXml::PARSER::IGNORE_CONVERSION_ERRORS );
	do_load();
	set_input_data();
	cgi::consistency_check( _dom.get_root() );
	return;
	M_EPILOG
}

OSecurityContext const& HApplication::get_default_security_context( void ) const {
	return ( _defaultSecurityContext );
}

HApplication::MODE HApplication::get_mode( void ) const {
	return ( _mode );
}

HApplication::HASH HApplication::get_hash( void ) const {
	return ( _hash );
}

yaal::hcore::HString const& HApplication::id( void ) const {
	M_PROLOG
	return ( _id );
	M_EPILOG
}

yaal::hcore::HString const& HApplication::code( void ) const {
	M_PROLOG
	return ( _code );
	M_EPILOG
}

yaal::hcore::HString const& HApplication::name( void ) const {
	M_PROLOG
	return ( _name );
	M_EPILOG
}

void HApplication::do_init( void ) {
	OUT << __PRETTY_FUNCTION__ << endl;
}

void HApplication::do_load( void ) {
	OUT << __PRETTY_FUNCTION__ << endl;
}

bool HApplication::handle_auth( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	return ( do_handle_auth( req_, session_ ) );
	M_EPILOG
}

bool HApplication::do_handle_auth( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	static char const AUTH[] = "auth";
	static char const LOGOUT[] = "logout";
	static char const LOGIN[] = "login";
	static char const PASSWORD[] = "password";
	static char const USERS[] = "users";
	ORequest::value_t action( req_.lookup( HACTION, ORequest::ORIGIN::POST ) );
	bool handled( false );
	if ( action ) {
		if ( *action == AUTH ) {
			ORequest::value_t login( req_.lookup( LOGIN, ORequest::ORIGIN::POST ) );
			ORequest::value_t password( req_.lookup( PASSWORD, ORequest::ORIGIN::POST ) );
			if ( login && password ) {
				HString queryString;
				try {
					queryString = (
						HFormat(
							"SELECT ( SELECT COUNT(*) FROM %1$s WHERE %2$s = LOWER(?) AND %3$s = LOWER(?) ) + ( SELECT COUNT(*) FROM %1$s WHERE %2$s = LOWER(?) );"
						) % _tableUser % _columnLogin % _columnPassword
					).string();
					OUT << "query: " << queryString << endl;
				} catch ( HFormatException const& e ) {
					OUT << e.what() << endl;
				}
				HQuery::ptr_t query( _db->prepare_query( queryString ) );
				query->bind( 1, *login );
				query->bind( 2, _hash == HASH::SHA1 ? tools::hash::sha1( *password ) : tools::hash::md5( *password ) );
				query->bind( 3, *login );
				HRecordSet::ptr_t rs( query->execute() );
				M_ENSURE( !! rs );
				HRecordSet::iterator row = rs->begin();
				if ( row == rs->end() ) {
					OUT << _db->get_error() << endl;
					M_ENSURE( ! "database execute_query error" );
				}
				int result( lexical_cast<int>( *row[0] ) );
				if ( result == 2 ) {
					session_.set_user( *login );
					session_.add_group( USERS );
					OUT << "authenticated user: " << *login << endl;
				} else if ( result == 1 ) {
					OUT << "invalid password" << endl;
				} else {
					OUT << "invalid user" << endl;
				}
			}
			handled = true;
		} else if ( *action == LOGOUT ) {
			OUT << "user: " << session_.get_user() << "logged out" << endl;
			sessions().erase( session_.get_id() );
			handled = true;
		}
	}
	return ( handled );
	M_EPILOG
}

bool HApplication::handle_forms( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	return ( do_handle_forms( req_, session_ ) );
	M_EPILOG
}

bool HApplication::do_handle_forms( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	ORequest::value_t action( req_.lookup( HACTION, ORequest::ORIGIN::POST ) );
	bool handled( false );
	if ( action ) {
		forms_t::iterator formIt( _forms.find( *action ) );
		if ( formIt != _forms.end() ) {
			if ( formIt->second->verify( req_, session_ ) ) {
				OUT << __PRETTY_FUNCTION__ << ": user input is valid" << endl;
				formIt->second->commit( req_, session_ );
			}
			handled = true;
		}
	}
	return ( handled );
	M_EPILOG
}

void HApplication::do_handle_logic( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	if ( ! handle_auth( req_, session_ ) ) {
		handle_forms( req_, session_ );
	}
	static char const LOGIC_PATH[] = "/html/logic/";
	HXml::HNodeSet logic( _dom.get_elements_by_path( LOGIC_PATH ) );
	if ( !logic.is_empty() ) {
		cgi::make_cookies( logic[0], req_ );
	}
	return;
	M_EPILOG
}

void HApplication::do_generate_page( ORequest const& req, HSession const& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << ( req.is_ssl() ? " with SSL" : "" ) << endl;
	cgi::default_t defaults;
	HXml::HNodeProxy root( dom().get_root() );
	if ( !! root ) {
		cgi::waste_children( root, req, defaults );
		cgi::apply_acl( root, req, _defaultSecurityContext, session_ );
		cgi::mark_children( root, req, defaults, dom() );
		cgi::move_children( root, req, dom() );
		cgi::expand_autobutton( root, req );
		cgi::show_messages( root, req );
		cgi::clean( root );
		cgi::substitute_variables( root, req, session_ );
		cgi::run_query( root, db(), dom() );
		cgi::fill_forms( this, root, session_ );
	}
	return;
	M_EPILOG
}

void HApplication::generate_page( ORequest const& req, HSession const& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	do_generate_page( req, session_ );
	if ( !! dom().get_root() ) {
		_dom.save( req.socket(), HXml::GENERATOR::STRIP_ENTITIES );
	}
	return;
	M_EPILOG
}

void HApplication::handle_logic( ORequest& req, HSession& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	do_handle_logic( req, session_ );
	return;
	M_EPILOG
}

HXml& HApplication::dom( void ) {
	return ( _dom );
}

yaal::dbwrapper::HDataBase::ptr_t HApplication::db( void ) {
	return ( do_db() );
}

yaal::dbwrapper::HDataBase::ptr_t HApplication::do_db( void ) {
	return ( _db );
}

HApplication::sessions_t const& HApplication::sessions( void ) const {
	return ( _sessions );
}

HApplication::sessions_t& HApplication::sessions( void ) {
	return ( _sessions );
}

void HApplication::add_form( forms_t::value_type&& form_ ) {
	M_PROLOG
	_forms.insert( yaal::move( form_ ) );
	return;
	M_EPILOG
}

void HApplication::set_input_data( void ) {
	M_PROLOG
	for ( forms_t::value_type& f : _forms ) {
		cgi::set_input_data( dom().get_element_by_id( f.first ), f.second );
	}
	return;
	M_EPILOG
}

void HApplication::fill_form( yaal::hcore::HString const& id_, HSession const& session_ ) {
	M_PROLOG
	forms_t::iterator fi( _forms.find( id_ ) );
	if ( fi != _forms.end() ) {
		fi->second->fill( session_ );
	}
	return;
	M_EPILOG
}

}

