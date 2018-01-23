/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_FORM_HXX_INCLUDED
#define HECTOR_FORM_HXX_INCLUDED 1

#include <yaal/hcore/harray.hxx>
#include <yaal/hcore/hresource.hxx>
#include <yaal/dbwrapper/hcruddescriptor.hxx>

#include "verificator.hxx"

namespace hector {

class HForm {
public:
	typedef yaal::hcore::HResource<HForm> ptr_t;
	typedef yaal::hcore::HResource<HVerificatorInterface> verificator_t;
	struct OInput {
		enum class TYPE {
			TEXT,
			PASSWORD,
			CALENDAR
		};
		typedef yaal::hcore::HBitFlag<struct FLAGS> flags_t;
		struct FLAGS {
			static flags_t const DEFAULT;
			static flags_t const NOT_EMPTY;
			static flags_t const OPTIONAL;
		};
		yaal::hcore::HString _column;
		yaal::hcore::HString _htmlName;
		yaal::hcore::HString* _data;
		TYPE _type;
		ACCESS::mode_t _mode;
		flags_t _flags;
		OInput( yaal::hcore::HString const& column_, yaal::hcore::HString const& htmlName_, TYPE type_, ACCESS::mode_t mode_, flags_t flags_ )
			: _column( column_ )
			, _htmlName( htmlName_ )
			, _data( nullptr )
			, _type( type_ )
			, _mode( mode_ )
			, _flags( flags_ ) {
			return;
		}
		OInput( OInput const& ) = default;
		OInput& operator = ( OInput const& ) = default;
	};
private:
	typedef yaal::hcore::HLookupMap<yaal::hcore::HString, OInput> inputs_t;
	typedef yaal::hcore::HLookupMap<yaal::hcore::HString, OInput const*> inputs_db_view_t;
	yaal::hcore::HString _id;
	yaal::hcore::HString _table;
	yaal::hcore::HString _filter;
	yaal::dbwrapper::HCRUDDescriptor::field_names_t _readColumns;
	yaal::dbwrapper::HCRUDDescriptor::field_names_t _writeColumns;
	inputs_t _inputs;
	inputs_db_view_t _inputsDBView;
	verificator_t _verificator;
	yaal::dbwrapper::HCRUDDescriptor _crud;
	HApplication& _application;
public:
	HForm( HApplication&, yaal::hcore::HString const&, yaal::hcore::HString const&, yaal::hcore::HString const& );
	void fill( HSession const& );
	void add_input( yaal::hcore::HString const&, yaal::hcore::HString const&, OInput::TYPE, ACCESS::mode_t, OInput::flags_t = OInput::FLAGS::DEFAULT );
	void set_input_data( yaal::hcore::HString const&, yaal::hcore::HString& );
	void set_verificator(
		HVerificatorInterface::TYPE,
		yaal::hcore::HString const&,
		cgi::params_t const&
	);
	bool verify( ORequest&, HSession& );
	void commit( ORequest&, HSession& );
	HApplication& app( void ) const {
		return ( _application );
	}
	void finalize( void );
	yaal::hcore::HString const& id( void ) const {
		return ( _id );
	}
private:
	HForm( HForm const& ) = delete;
	HForm& operator = ( HForm const& ) = delete;
};

}

#endif /* #ifndef HECTOR_FORM_HXX_INCLUDED */

