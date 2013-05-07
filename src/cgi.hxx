/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	cgi.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_CGI_HXX_INCLUDED
#define HECTOR_CGI_HXX_INCLUDED 1

#include <yaal/tools/hxml.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>

#include "orequest.hxx"
#include "securitycontext.hxx"
#include "session.hxx"

namespace	hector {

class HApplication;

namespace cgi {

typedef yaal::hcore::HSet<yaal::hcore::HString> keep_t;
typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> default_t;
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
void move_children( yaal::tools::HXml::HNodeProxy, ORequest const&,
		yaal::tools::HXml&, yaal::tools::HXml::HNodeProxy* = NULL );
void run_query( yaal::tools::HXml::HNodeProxy, yaal::dbwrapper::HDataBase::ptr_t,
		yaal::tools::HXml&, yaal::tools::HXml::HNodeProxy* = NULL );
void prepare_logic( HApplication*, yaal::tools::HXml::HNodeProxy );
void make_cookies( yaal::tools::HXml::HNodeProxy, ORequest& );
void consistency_check( yaal::tools::HXml::HNodeProxy );
bool has_access( ACCESS::type_t, HSession const&, OSecurityContext const& );
void clean( yaal::tools::HXml::HNodeProxy );

}

}

#endif /* not HECTOR_CGI_HXX_INCLUDED */

