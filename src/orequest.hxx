/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.hxx - this file is integral part of `hector' project.

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

#ifndef HECTOR_OREQUEST_HXX_INCLUDED
#define HECTOR_OREQUEST_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hpointer.hxx>
#include <yaal/hcore/hstrongenum.hxx>
#include <yaal/hcore/hmap.hxx>
#include <yaal/hcore/hsocket.hxx>
#include <yaal/tools/hoptional.hxx>

namespace hector {

class ORequest {
	typedef ORequest this_type;
public:
	typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> dictionary_t;
	typedef yaal::hcore::HPointer<dictionary_t> dictionary_ptr_t;
	struct ORIGIN {
		typedef enum {
			NONE = 0,
			ENV = 1,
			COOKIE = 2,
			GET = 4,
			POST = 8,
			JAR = 16,
			ANY = 31
		} enum_t;
	};
	typedef yaal::hcore::HStrongEnum<ORIGIN> origin_t;
	class HConstIterator;
	typedef HConstIterator const_iterator;
	typedef yaal::tools::HOptional<yaal::hcore::HString const&> value_t;
private:
	yaal::hcore::HSocket::ptr_t _socket;
	dictionary_ptr_t _environment;
	dictionary_ptr_t _gET;
	dictionary_ptr_t _pOST;
	dictionary_ptr_t _cookies;
	dictionary_ptr_t _jar;
public:
	ORequest( yaal::hcore::HSocket::ptr_t = yaal::hcore::HSocket::ptr_t() );
	ORequest( ORequest const& );
	ORequest& operator = ( ORequest const& );
	void update( yaal::hcore::HString const&, yaal::hcore::HString const&, origin_t const& );
	bool lookup( yaal::hcore::HString const&, yaal::hcore::HString&, origin_t const& = ORIGIN::ANY ) const;
	value_t lookup( yaal::hcore::HString const&, origin_t const& = ORIGIN::ANY ) const;
	void decompress_jar( yaal::hcore::HString const& );
	dictionary_ptr_t compress_jar( yaal::hcore::HString const& );
	bool is_ssl( void ) const;
	yaal::hcore::HSocket::ptr_t socket( void );
	yaal::hcore::HSocket::ptr_t const socket( void ) const;
	const_iterator begin( void ) const;
	const_iterator end( void ) const;
	friend class HConstIterator;
};

typedef yaal::hcore::HExceptionT<ORequest> ORequestException;

class ORequest::HConstIterator {
	ORequest const* _owner;
	ORequest::origin_t _origin;
	dictionary_t::const_iterator _it;
public:
	HConstIterator( HConstIterator const& );
	HConstIterator& operator = ( HConstIterator const& );
	bool operator != ( HConstIterator const& ) const;
	HConstIterator& operator ++ ( void );
	ORequest::dictionary_t::value_type const& operator* ( void ) const;
private:
	HConstIterator( ORequest const*, ORequest::origin_t const&, ORequest::dictionary_t::const_iterator );
	friend class ORequest;
};

}

#endif /* not HECTOR_OREQUEST_HXX_INCLUDED */

