/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	form.cxx - this file is integral part of `hector' project.

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

#include <yaal/hcore/macro.hxx>
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

HForm::HForm( HApplication& application_, yaal::hcore::HString const& table_, yaal::hcore::HString const& filter_ )
	: _table( table_ )
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

void HForm::add_input( yaal::hcore::HString const& name_, yaal::hcore::HString const& column_, OInput::TYPE type_, ACCESS::mode_t mode_ ) {
	M_PROLOG
	_inputs.insert( make_pair( name_, OInput( column_, name_, type_, mode_ ) ) );
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
	_crud.set_columns( _writeColumns );
	_crud.set_filter_value( session_.get_user() );
	int colNo( 0 );
	for ( HString const& col : _writeColumns ) {
		OInput const& input( *_inputsDBView.at( col ) );
		OUT << "req ask: " << input._htmlName << endl;
		ORequest::value_t value( req_.lookup( input._htmlName, ORequest::ORIGIN::POST ) );
		if ( !! value ) {
			_crud[colNo] = ( input._type == OInput::TYPE::PASSWORD ) ? tools::hash::sha1( *value ) : *value;
		} else {
			_crud[colNo] = HRecordSet::value_t();
		}
		++ colNo;
	}
	HRecordSet::ptr_t rs( _crud.execute( HCRUDDescriptor::MODE::UPDATE ) );
	return;
	M_EPILOG
}

}

