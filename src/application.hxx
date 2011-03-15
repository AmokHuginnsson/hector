/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	application.hxx - this file is integral part of `hector' project.

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

#ifndef APPLICATION_HXX_INCLUDED
#define APPLICATION_HXX_INCLUDED

#include <yaal/tools/hxml.hxx>
#include <yaal/tools/hplugin.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>
#include "server.hxx"
#include "securitycontext.hxx"
#include "form.hxx"

namespace hector
{

struct HSession;

class HApplication
	{
public:
	typedef HApplication this_type;
	typedef yaal::hcore::HMap<yaal::hcore::HString, HSession> sessions_t;
	typedef yaal::hcore::HMap<yaal::hcore::HString, HForm> forms_t;
	typedef yaal::hcore::HHashSet<yaal::hcore::HString> verificators_t;
private:
	yaal::tools::HXml _dOM;
	yaal::hcore::HString _name;
	OSecurityContext _defaultSecurityContext;
	sessions_t _sessions;
	yaal::dbwrapper::HDataBase::ptr_t _db;
	forms_t _forms;
	verificators_t _verificators;
public:
	typedef yaal::hcore::HPointer<HApplication> ptr_t;
	HApplication( yaal::dbwrapper::HDataBase::ptr_t );
	virtual ~HApplication( void );
	void load( yaal::hcore::HString const&, yaal::hcore::HString const& );
	void handle_logic( ORequest&, HSession& );
	void generate_page( ORequest const&, HSession const& );
	sessions_t& sessions( void );
	sessions_t const& sessions( void ) const;
	yaal::dbwrapper::HDataBase::ptr_t db( void );
	void add_verificator( yaal::hcore::HString const& );
	void add_form( forms_t::value_type const& );
protected:
	virtual void do_load( void );
	virtual void do_handle_logic( ORequest&, HSession& ) = 0;
	virtual void do_generate_page( ORequest const&, HSession const& ) = 0;
	virtual yaal::dbwrapper::HDataBase::ptr_t do_db( void );
	void handle_auth( ORequest&, HSession& );
	virtual void do_handle_auth( ORequest&, HSession& );
	yaal::tools::HXml& dom( void );
private:
	HApplication( HApplication const& );
	HApplication& operator = ( HApplication const& );
	};

typedef yaal::hcore::HExceptionT<HApplication> HApplicationException;

}

#endif /* not APPLICATION_HXX_INCLUDED */

