/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_CGI_HXX_INCLUDED
#define HECTOR_CGI_HXX_INCLUDED 1

#include <yaal/tools/hxml.hxx>
#include <yaal/hcore/hregex.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>

#include "orequest.hxx"
#include "securitycontext.hxx"
#include "session.hxx"

namespace	hector {

class HForm;
class HApplication;

namespace cgi {

class HParameter {
public:
	typedef yaal::hcore::HString (*transform_t)( yaal::hcore::HString const& );
private:
	yaal::hcore::HString _name;
	transform_t _transform;
public:
	HParameter( yaal::hcore::HString const& name_, transform_t transform_ = nullptr )
		: _name( name_ )
		, _transform( transform_ ) {
		return;
	}
	HParameter( HParameter const& ) = default;
	HParameter( HParameter&& ) = default;
	HParameter& operator = ( HParameter const& ) = default;
	HParameter& operator = ( HParameter&& ) = default;
	yaal::hcore::HString const& name( void ) const {
		return ( _name );
	}
	yaal::hcore::HString transform( yaal::hcore::HString const& value_ ) const {
		return ( _transform ? _transform( value_ ) : value_ );
	}
};

class HReplacer {
private:
	yaal::hcore::HRegex _regex;
	yaal::hcore::HRegex::replacer_t _replcer;
public:
	HReplacer( yaal::hcore::HString const& pattern_, yaal::hcore::HRegex::replacer_t replacer_ )
		: _regex( pattern_ )
		, _replcer( replacer_ ) {
	}
	yaal::hcore::HRegex& regex( void ) {
		return ( _regex );
	}
	yaal::hcore::HRegex::replacer_t& replcer( void ) {
		return ( _replcer );
	}
};

typedef yaal::hcore::HSet<yaal::hcore::HString> keep_t;
typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> default_t;
typedef yaal::hcore::HArray<yaal::hcore::HString> strings_t;
typedef yaal::hcore::HArray<HParameter> params_t;
typedef yaal::hcore::HResource<HForm> form_t;
bool is_in_attribute( yaal::tools::HXml::HNode::properties_t const&, yaal::hcore::HString const&, yaal::hcore::HString const& );
bool is_kind_of( yaal::tools::HXml::HNode::properties_t const&, yaal::hcore::HString const& );
bool has_attribute( yaal::tools::HXml::HNode::properties_t const&, yaal::hcore::HString const& );
bool is_in_attribute( yaal::tools::HXml::HConstNodeProxy const&, yaal::hcore::HString const&, yaal::hcore::HString const& );
bool is_kind_of( yaal::tools::HXml::HConstNodeProxy const&, yaal::hcore::HString const& );
bool has_attribute( yaal::tools::HXml::HConstNodeProxy const&, yaal::hcore::HString const& );
void waste_children( yaal::tools::HXml::HNodeProxy, ORequest const&,
		default_t&, yaal::tools::HXml::HNodeProxy* = NULL );
void apply_acl( yaal::tools::HXml::HNodeProxy, ORequest const&,
		OSecurityContext const&, HSession const&, yaal::tools::HXml::HNodeProxy* = NULL );
void mark_children( yaal::tools::HXml::HNodeProxy, ORequest const&,
		default_t const&, yaal::tools::HXml& );
void expand_autobutton( yaal::tools::HXml::HNodeProxy, ORequest const& );
void show_messages( yaal::tools::HXml::HNodeProxy, ORequest const& );
void move_children( yaal::tools::HXml::HNodeProxy, ORequest const&,
		yaal::tools::HXml&, yaal::tools::HXml::HNodeProxy* = NULL );
void run_query( yaal::tools::HXml::HNodeProxy, yaal::dbwrapper::HDataBase::ptr_t,
		yaal::tools::HXml&, yaal::tools::HXml::HNodeProxy* = NULL );
void fill_forms( HApplication*, yaal::tools::HXml::HNodeProxy, HSession const& );
void substitute_variables( yaal::tools::HXml::HNodeProxy, ORequest const&, HSession const&, HReplacer* = nullptr );
void set_input_data( yaal::tools::HXml::HNodeProxy, form_t& );
void prepare_logic( HApplication*, yaal::tools::HXml::HNodeProxy );
void make_cookies( yaal::tools::HXml::HNodeProxy, ORequest& );
void consistency_check( yaal::tools::HXml::HNodeProxy );
bool has_access( ACCESS::type_t, HSession const&, OSecurityContext const& );
void clean( yaal::tools::HXml::HNodeProxy );

}

struct CGI {};
typedef yaal::hcore::HExceptionT<CGI> HCGIException;

}

#endif /* not HECTOR_CGI_HXX_INCLUDED */

