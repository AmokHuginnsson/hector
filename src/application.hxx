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
#include "server.hxx"

namespace hector
{

struct OSession;

class HApplication
	{
	typedef HApplication this_type;
private:
	yaal::tools::HXml _dOM;
	yaal::hcore::HString _name;
public:
	typedef yaal::hcore::HPointer<HApplication> ptr_t;
	HApplication( void );
	virtual ~HApplication( void );
	void load( yaal::hcore::HString const&, yaal::hcore::HString const& );
	void handle_logic( ORequest&, OSession& );
	void generate_page( ORequest const&, OSession const& );
protected:
	virtual void do_load( void );
	virtual void do_handle_logic( ORequest&, OSession& ) = 0;
	virtual void do_generate_page( ORequest const&, OSession const& ) = 0;
	yaal::tools::HXml& dom( void );
private:
	HApplication( HApplication const& );
	HApplication& operator = ( HApplication const& );
	};

typedef yaal::hcore::HExceptionT<HApplication> HApplicationException;

}

#endif /* not APPLICATION_HXX_INCLUDED */

