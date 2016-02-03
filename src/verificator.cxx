/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  verificator.cxx - this file is integral part of `hector' project.

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

#include <yaal/tools/hstringstream.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>

#include "verificator.hxx"
M_VCSID( "$Id: " __ID__ " $" )
M_VCSID( "$Id: " __TID__ " $" )
#include "setup.hxx"
#include "form.hxx"
#include "application.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

HVerificatorInterface::HVerificatorInterface( cgi::params_t const& params_, HForm* form_ )
	: _params( params_ )
	, _form( form_ ) {
	return;
}

bool HVerificatorInterface::verify( ORequest const& req_, HSession& session_ ) {
	return ( do_verify( req_, session_ ) );
}

HHuginnVerificator::HHuginnVerificator(
	yaal::hcore::HString const& code_,
	cgi::params_t const& params_,
	HForm* form_
) : HVerificatorInterface( params_, form_ )
	, _huginn() {
	M_PROLOG
	HHuginn::ptr_t h( make_pointer<HHuginn>() );
	HStringStream s( code_ );
	h->load( s );
	h->preprocess();
	if ( h->parse() && h->compile() ) {
		_huginn = h;
	} else {
		throw HRuntimeException( h->error_message() );
	}
	return;
	M_EPILOG
}

bool HHuginnVerificator::do_verify( ORequest const& req_, HSession& session_ ) {
	_huginn->clear_arguments();
	for ( cgi::HParameter const& p : _params ) {
		HString const& name( p.name() );
		if ( name.front() == '@' ) {
			if ( name == "@user" ) {
				_huginn->add_argument( session_.get_user() );
				out << "setting param: " << name << ", to value: " << session_.get_user() << endl;
			}
		} else {
			ORequest::value_t value( req_.lookup( name, ORequest::ORIGIN::POST ) );
			if ( !! value ) {
				_huginn->add_argument( *value );
				out << "setting param: " << name << ", to value: " << *value << endl;
			}
		}
	}
	if ( ! _huginn->execute() ) {
		throw HRuntimeException( _huginn->error_message() );
	}
	HHuginn::value_t const& result( _huginn->result() );
	if ( result->type_id() != HHuginn::TYPE::BOOLEAN ) {
		throw HRuntimeException( "bad result type from verificator" );
	}
	return ( static_cast<HHuginn::HBoolean const*>( result.raw() )->value() );
}

HSQLVerificator::HSQLVerificator(
	yaal::hcore::HString const& code_,
	cgi::params_t const& params_,
	HForm* form_
) : HVerificatorInterface( params_, form_ )
	, _query( form_->app().db()->prepare_query( HString( code_ ).trim() ) ) {
	M_PROLOG
	return;
	M_EPILOG
}

bool HSQLVerificator::do_verify( ORequest const& req_, HSession& session_ ) {
	int paramNo( 1 );
	for ( cgi::HParameter const& p : _params ) {
		HString const& name( p.name() );
		if ( name.front() == '@' ) {
			if ( name == "@user" ) {
				_query->bind( paramNo, session_.get_user() );
				out << "setting param: " << name << ", to value: " << session_.get_user() << endl;
			}
		} else {
			ORequest::value_t value( req_.lookup( name, ORequest::ORIGIN::POST ) );
			if ( !! value ) {
				HString realValue( p.transform( *value ) );
				_query->bind( paramNo, realValue );
				out << "setting param: " << name << ", to value: " << realValue << endl;
			}
		}
		++ paramNo;
	}
	HRecordSet::ptr_t result( _query->execute() );
	HRecordSet::HIterator rowIt( result->begin() );
	bool ok( false );
	if ( rowIt != result->end() ) {
		if ( !!rowIt[0]  ) {
			if ( *rowIt[0] == "ok" ) {
				ok = true;
			}
			out << __PRETTY_FUNCTION__ << ": verificator returned: " << *rowIt[0] << endl;
		} else {
			out << __PRETTY_FUNCTION__ << ": NULL value returned in result set" << endl;
		}
	} else {
		out << __PRETTY_FUNCTION__ << ": empty result set" << endl;
	}
	return ( ok );
}

}

