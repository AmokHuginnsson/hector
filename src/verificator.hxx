/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  verificator.hxx - this file is integral part of `hector' project.

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

/*! \file verificator.hxx
 * \brief Declaration HVerificatorInterface, HHuginnVerificator and HSQLVerificator.
 */

#ifndef HECTOR_VERIFICATOR_HXX_INCLUDED
#define HECTOR_VERIFICATOR_HXX_INCLUDED 1

#include <yaal/tools/hhuginn.hxx>
#include <yaal/tools/hstringstream.hxx>
#include <yaal/dbwrapper/hquery.hxx>

#include "cgi.hxx"

namespace hector {

class HSession;
class HForm;

class HVerificatorInterface {
public:
	enum class TYPE {
		NONE,
		HUGINN,
		SQL
	};
	typedef HVerificatorInterface this_type;
	virtual ~HVerificatorInterface( void ) {}
protected:
	cgi::params_t _params;
	HForm* _form;
public:
	HVerificatorInterface( cgi::params_t const&, HForm* );
	bool verify( ORequest&, HSession& );
private:
	virtual bool do_verify( ORequest&, HSession& ) = 0;
	HVerificatorInterface( HVerificatorInterface const& ) = delete;
	HVerificatorInterface& operator = ( HVerificatorInterface const& ) = delete;
};

class HHuginnVerificator : public HVerificatorInterface {
public:
	typedef HHuginnVerificator this_type;
	typedef HVerificatorInterface base_type;
private:
	yaal::tools::HHuginn::ptr_t _huginn;
	yaal::tools::HStringStream _output;
	virtual bool do_verify( ORequest&, HSession& ) override;
public:
	HHuginnVerificator( yaal::hcore::HString const&, cgi::params_t const&, HForm* );
};

class HSQLVerificator : public HVerificatorInterface {
public:
	typedef HSQLVerificator this_type;
	typedef HVerificatorInterface base_type;
private:
	yaal::dbwrapper::HQuery::ptr_t _query;
public:
	HSQLVerificator( yaal::hcore::HString const&, cgi::params_t const&, HForm* );
private:
	virtual bool do_verify( ORequest&, HSession& ) override;
};

}

#endif /* #ifndef HECTOR_VERIFICATOR_HXX_INCLUDED */

