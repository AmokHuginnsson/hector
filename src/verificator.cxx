/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <yaal/hcore/hclock.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>
#include <yaal/tools/huginn/boolean.hxx>

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

bool HVerificatorInterface::verify( ORequest& req_, HSession& session_ ) {
	HClock c;
	bool result( do_verify( req_, session_ ) );
	OUT << __PRETTY_FUNCTION__ << ": verificator executed in " << c.get_time_elapsed( time::UNIT::MICROSECOND ) << " microseconds" << endl;
	return result;
}

HHuginnVerificator::HHuginnVerificator(
	yaal::hcore::HString const& code_,
	cgi::params_t const& params_,
	HForm* form_
) : HVerificatorInterface( params_, form_ )
	, _huginn()
	, _output() {
	M_PROLOG
	HHuginn::ptr_t h( make_pointer<HHuginn>() );
	HStringStream s( code_ );
	h->set_output_stream( _output );
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

bool HHuginnVerificator::do_verify( ORequest& req_, HSession& session_ ) {
	_huginn->clear_arguments();
	for ( cgi::HParameter const& p : _params ) {
		HString const& name( p.name() );
		if ( name.front() == '@' ) {
			if ( name == "@user" ) {
				_huginn->add_argument( session_.get_user() );
				OUT << "setting param: " << name << ", to value: " << session_.get_user() << endl;
			}
		} else {
			ORequest::value_t value( req_.lookup( name, ORequest::ORIGIN::POST ) );
			if ( !! value ) {
				_huginn->add_argument( p.transform( *value ) );
				OUT << "setting param: " << name << ", to value: " << *value << endl;
			}
		}
	}
	_output.reset();
	if ( ! _huginn->execute() ) {
		throw HRuntimeException( _huginn->error_message() );
	}
	HHuginn::value_t const& result( _huginn->result() );
	if ( result->type_id() != HHuginn::TYPE::BOOLEAN ) {
		throw HRuntimeException( "bad result type from verificator" );
	}
	bool ok( static_cast<huginn::HBoolean const*>( result.raw() )->value() );
	if ( ! _output.str().is_empty() ) {
		req_.message( _form->id(), ok ? LOG_LEVEL::INFO : LOG_LEVEL::ERROR, _output.consume() );
	}
	return ok;
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

bool HSQLVerificator::do_verify( ORequest& req_, HSession& session_ ) {
	int paramNo( 1 );
	for ( cgi::HParameter const& p : _params ) {
		HString const& name( p.name() );
		if ( name.front() == '@' ) {
			if ( name == "@user" ) {
				_query->bind( paramNo, session_.get_user() );
				OUT << "setting param: " << name << ", to value: " << session_.get_user() << endl;
			}
		} else {
			ORequest::value_t value( req_.lookup( name, ORequest::ORIGIN::POST ) );
			if ( !! value ) {
				HString realValue( p.transform( *value ) );
				_query->bind( paramNo, realValue );
				OUT << "setting param: " << name << ", to value: " << realValue << endl;
			}
		}
		++ paramNo;
	}
	HRecordSet::ptr_t result( _query->execute() );
	HRecordSet::HIterator rowIt( result->begin() );
	bool ok( false );
	if ( rowIt != result->end() ) {
		if ( !!rowIt[0]  ) {
			HString res( *rowIt[0] );
			if ( res.find( "ok:" ) == 0 ) {
				ok = true;
				res.shift_left( 3 );
			}
			req_.message( _form->id(), ok ? LOG_LEVEL::INFO : LOG_LEVEL::ERROR, res );
		} else {
			req_.message( _form->id(), LOG_LEVEL::ALERT, "Result set error (NULL returned)" );
		}
	} else {
		req_.message( _form->id(), LOG_LEVEL::ALERT, "Result set error (empty result set)" );
	}
	return ok;
}

}

