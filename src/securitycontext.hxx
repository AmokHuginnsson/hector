/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED
#define HECTOR_SECURITYCONTEXT_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>

namespace hector {

struct ACCESS {
	typedef enum : yaal::u16_t {
		READ = 04,
		WRITE = 02,
		EXECUTE = 01
	} type_t;
	typedef enum : yaal::u16_t {
		USER = 6,
		GROUP = 3,
		OTHER = 0
	} context_t;
	typedef enum : yaal::u16_t {
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
	} mode_t;
};

struct OSecurityContext {
	yaal::hcore::HString _user;
	yaal::hcore::HString _group;
	ACCESS::mode_t _mode;
	OSecurityContext( void )
		: _user()
		, _group()
		, _mode( ACCESS::NONE ) {
	}
};

}

#endif /* #ifndef HECTOR_SECURITYCONTEXT_HXX_INCLUDED */

