/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.h - this file is integral part of `hector' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You can not use any part of sources of this software.
  2. You can not redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you can not demand any fees
     for this software.
     You can not even demand cost of the carrier (CD for example).
  5. You can not include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#ifndef __OREQUEST_H
#define __OREQUEST_H

namespace hector
{

class ORequest
	{
public:
	typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> dictionary_t;
	typedef yaal::hcore::HPointer<dictionary_t> dictionary_ptr_t;
	struct ORIGIN
		{
		typedef enum
			{
			D_NONE = 0,
			D_ENV = 1,
			D_COOKIE = 2,
			D_GET = 4,
			D_POST = 8,
			D_ANY = 15
			} origin_t;
		};
private:
	yaal::hcore::HSocket::ptr_t f_oSocket;
	dictionary_ptr_t f_oEnvironment;
	dictionary_ptr_t f_oCookies;
	dictionary_ptr_t f_oGET;
	dictionary_ptr_t f_oPOST;
public:
	ORequest( yaal::hcore::HSocket::ptr_t );
	ORequest( ORequest const& );
	ORequest& operator = ( ORequest const& );
	void update( yaal::hcore::HString const&, yaal::hcore::HString const&, ORIGIN::origin_t const& );
	bool lookup( yaal::hcore::HString const&, yaal::hcore::HString&, ORIGIN::origin_t const& = ORIGIN::D_ANY ) const;
	yaal::hcore::HSocket::ptr_t socket( void );
	};

}

#endif /* not __OREQUEST_H */

