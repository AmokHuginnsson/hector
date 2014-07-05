/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	securitycontext.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED
#define HECTOR_SECURITYCONTEXT_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hstrongenum.hxx>

namespace hector {

struct ACCESS {
	typedef enum {
		READ = 04,
		WRITE = 02,
		EXECUTE = 01
	} type_t;
	typedef enum {
		USER = 6,
		GROUP = 3,
		OTHER = 0
	} context_t;
	typedef enum {
		USER_READ = READ << 6,
		USER_WRITE = WRITE << 6,
		USER_EXECUTE = EXECUTE << 6,
		GROUP_READ = READ << 3,
		GROUP_WRITE = WRITE << 3,
		GROUP_EXECUTE = EXECUTE << 3,
		OTHER_READ = READ,
		OTHER_WRITE = WRITE,
		OTHER_EXECUTE = EXECUTE,
		NONE = 0
	} enum_t;
};

typedef yaal::hcore::HStrongEnum<ACCESS> access_type_t;

struct OSecurityContext {
	yaal::hcore::HString _user;
	yaal::hcore::HString _group;
	access_type_t _mode;
	OSecurityContext( void ) : _user(), _group(), _mode( ACCESS::NONE ) {}
};

}

#endif /* #ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED */

