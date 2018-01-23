/* Read hector/LICENSE.md file for copyright and licensing information. */

#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/hash.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "form.hxx"
#include "setup.hxx"
#include "application.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

HForm::OInput::flags_t const HForm::OInput::FLAGS::DEFAULT = HForm::OInput::flags_t::new_flag();
HForm::OInput::flags_t const HForm::OInput::FLAGS::NOT_EMPTY = HForm::OInput::flags_t::new_flag();
HForm::OInput::flags_t const HForm::OInput::FLAGS::OPTIONAL = HForm::OInput::flags_t::new_flag();

HForm::HForm(
	HApplication& application_,
	yaal::hcore::HString const& id_,
	yaal::hcore::HString const& table_,
	yaal::hcore::HString const& filter_
) : _id( id_ )
	, _table( table_ )
	, _filter( filter_ )
	, _readColumns()
	, _writeColumns()
	, _inputs()
	, _inputsDBView()
	, _verificator()
	, _crud( application_.db() )
	, _application( application_ ) {
	return;
}

void HForm::set_verificator(
	HVerificatorInterface::TYPE type_,
	yaal::hcore::HString const& verificator_,
	cgi::params_t const& params_
) {
	M_PROLOG
	if ( type_ == HVerificatorInterface::TYPE::HUGINN ) {
		_verificator.reset( new HHuginnVerificator( verificator_, params_, this ) );
	} else if ( type_ == HVerificatorInterface::TYPE::SQL ) {
		_verificator.reset( new HSQLVerificator( verificator_, params_, this ) );
	}
	return;
	M_EPILOG
}

void HForm::add_input( yaal::hcore::HString const& name_, yaal::hcore::HString const& column_, OInput::TYPE type_, ACCESS::mode_t mode_, OInput::flags_t flags_ ) {
	M_PROLOG
	_inputs.insert( make_pair( name_, OInput( column_, name_, type_, mode_, flags_ ) ) );
	return;
	M_EPILOG
}

void HForm::set_input_data( yaal::hcore::HString const& name_, yaal::hcore::HString& value_ ) {
	M_PROLOG
	inputs_t::iterator ii( _inputs.find( name_ ) );
	if ( ii != _inputs.end() ) {
		ii->second._data = &value_;
	}
	return;
	M_EPILOG
}

void HForm::finalize( void ) {
	M_PROLOG
	HString columns;
	for ( inputs_t::value_type const& input : _inputs ) {
		bool bean( false );
		if ( input.second._mode & ACCESS::USER_READ ) {
			_readColumns.push_back( input.second._column );
			bean = true;
		}
		if ( input.second._mode & ACCESS::USER_WRITE ) {
			_writeColumns.push_back( input.second._column );
			bean = true;
		}
		if ( bean ) {
			_inputsDBView.insert( make_pair( input.second._column, &input.second ) );
		}
	}
	_crud.set_table( _table );
	_crud.set_filter( _filter );
	return;
	M_EPILOG
}

void HForm::fill( HSession const& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	_crud.set_columns( _readColumns );
	_crud.set_filter_value( session_.get_user() );
	HRecordSet::ptr_t rs( _crud.execute( HCRUDDescriptor::MODE::READ ) );
	HRecordSet::HIterator rowIt( rs->begin() );
	if ( rowIt != rs->end() ) {
		for ( int i( 0 ), fc( rs->get_field_count() ); i < fc; ++ i ) {
			HRecordSet::value_t v( rowIt[i] );
			if ( !! v ) {
				OInput const& input( *_inputsDBView.at( rs->get_column_name( i ) ) );
				*(input._data) = *v;
			}
		}
	}
	return;
	M_EPILOG
}

bool HForm::verify( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	bool verified( false );
	if ( !! _verificator ) {
		verified = _verificator->verify( req_, session_ );
	}
	return ( verified );
	M_EPILOG
}

void HForm::commit( ORequest& req_, HSession& session_ ) {
	M_PROLOG
	OUT << __PRETTY_FUNCTION__ << endl;
	yaal::dbwrapper::HCRUDDescriptor::field_names_t writeColumns;
	bool ok( true );
	for ( HString const& col : _writeColumns ) {
		OInput const& input( *_inputsDBView.at( col ) );
		OUT << "req ask: " << input._htmlName << endl;
		ORequest::value_t value( req_.lookup( input._htmlName, ORequest::ORIGIN::POST ) );
		bool empty( ! value || value->is_empty() );
		if ( ! empty || ! ( input._flags & OInput::FLAGS::NOT_EMPTY ) ) {
			writeColumns.push_back( col );
		} else if ( ! ( input._flags & OInput::FLAGS::OPTIONAL ) )  {
			req_.message( _id, LOG_LEVEL::ERROR, "Field `"_ys.append( input._htmlName ).append( "' must be non-empty." ) );
			ok = false;
			break;
		}
	}
	if ( ok ) {
		_crud.set_columns( writeColumns );
		_crud.set_filter_value( session_.get_user() );
		int colNo( 0 );
		for ( HString const& col : writeColumns ) {
			OInput const& input( *_inputsDBView.at( col ) );
			ORequest::value_t value( req_.lookup( input._htmlName, ORequest::ORIGIN::POST ) );
			if ( !! value ) {
				_crud[colNo] = ( input._type == OInput::TYPE::PASSWORD ) ? ( _application.get_hash() == HApplication::HASH::SHA1 ? tools::hash::sha1( *value ) : tools::hash::md5( *value ) ) : *value;
			} else {
				_crud[colNo] = HRecordSet::value_t();
			}
			++ colNo;
		}
		HRecordSet::ptr_t rs( _crud.execute( HCRUDDescriptor::MODE::UPDATE ) );
	}
	return;
	M_EPILOG
}

}

