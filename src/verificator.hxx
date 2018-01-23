/* Read hector/LICENSE.md file for copyright and licensing information. */

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

