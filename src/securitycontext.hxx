/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	securitycontext.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED
#define HECTOR_SECURITYCONTEXT_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>

namespace hector
{

struct ACCESS
	{
	typedef enum
		{
		READ = 04,
		WRITE = 02,
		EXECUTE = 01
		} type_t;
	typedef enum
		{
		USER_READ = READ << 8,
		USER_WRITE = WRITE << 8,
		USER_EXECUTE = EXECUTE << 8,
		GROUP_READ = READ << 4,
		GROUP_WRITE = WRITE << 4,
		GROUP_EXECUTE = EXECUTE << 4,
		OTHER_READ = READ,
		OTHER_WRITE = WRITE,
		OTHER_EXECUTE = EXECUTE,
		NONE = 0
		} enum_t;
	};

typedef yaal::hcore::HStrongEnum<ACCESS> access_type_t;

struct OSecurityContext
	{
	yaal::hcore::HString _user;
	yaal::hcore::HString _group;
	access_type_t _mode;
	OSecurityContext( void ) : _user(), _group(), _mode( ACCESS::NONE ) {}
	};

}

#endif /* #ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED */

