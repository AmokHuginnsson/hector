/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.hxx - this file is integral part of `hector' project.

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

#ifndef OREQUEST_HXX_INCLUDED
#define OREQUEST_HXX_INCLUDED

namespace hector
{

class ORequest
	{
	typedef ORequest self_t;
public:
	typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> dictionary_t;
	typedef yaal::hcore::HPointer<dictionary_t> dictionary_ptr_t;
	struct ORIGIN
		{
		typedef enum
			{
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
private:
	yaal::hcore::HSocket::ptr_t f_oSocket;
	dictionary_ptr_t f_oEnvironment;
	dictionary_ptr_t f_oGET;
	dictionary_ptr_t f_oPOST;
	dictionary_ptr_t f_oCookies;
	dictionary_ptr_t f_oJar;
public:
	ORequest( yaal::hcore::HSocket::ptr_t = yaal::hcore::HSocket::ptr_t() );
	ORequest( ORequest const& );
	ORequest& operator = ( ORequest const& );
	void update( yaal::hcore::HString const&, yaal::hcore::HString const&, origin_t const& );
	bool lookup( yaal::hcore::HString const&, yaal::hcore::HString&, origin_t const& = ORIGIN::ANY ) const;
	void decompress_jar( yaal::hcore::HString const& );
	dictionary_ptr_t compress_jar( yaal::hcore::HString const& );
	yaal::hcore::HSocket::ptr_t socket( void );
	yaal::hcore::HSocket::ptr_t const socket( void ) const;
	const_iterator begin( void ) const;
	const_iterator end( void ) const;
	friend class HConstIterator;
	};

typedef yaal::hcore::HExceptionT<ORequest> ORequestException;

class ORequest::HConstIterator
	{
	ORequest const* f_poOwner;
	ORequest::origin_t f_eOrigin;
	dictionary_t::const_iterator f_oIt;
public:
	HConstIterator( HConstIterator const& );
	bool operator != ( HConstIterator const& ) const;
	HConstIterator& operator ++ ( void );
	ORequest::dictionary_t::map_elem_t const& operator* ( void ) const;
private:
	HConstIterator( ORequest const*, ORequest::origin_t const&, ORequest::dictionary_t::const_iterator );
	friend class ORequest;
	};

}

#endif /* not OREQUEST_HXX_INCLUDED */

