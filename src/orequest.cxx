/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.cxx - this file is integral part of `hector' project.

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

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "orequest.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector
{

ORequest::ORequest( HSocket::ptr_t socket_ )
	: _socket( socket_ ),
	_environment( new dictionary_t() ),
	_gET( new dictionary_t() ),
	_pOST( new dictionary_t() ),
	_cookies( new dictionary_t() ),
	_jar( new dictionary_t() )
	{
	}

ORequest::ORequest( ORequest const& req )
	: _socket( req._socket ),
	_environment( req._environment ),
	_gET( req._gET ), _pOST( req._pOST ),
	_cookies( req._cookies ), _jar( req._jar )
	{
	}

ORequest& ORequest::operator = ( ORequest const& req )
	{
	if ( &req != this )
		{
		_socket = req._socket;
		_environment = req._environment;
		_gET = req._gET;
		_pOST = req._pOST;
		_cookies = req._cookies;
		_jar = req._jar;
		}
	return ( *this );
	}

void ORequest::update( HString const& key, HString const& value, origin_t const& origin )
	{
	M_PROLOG
	dictionary_t* dict = NULL;
	switch ( origin.value() )
		{
		case ( ORIGIN::ENV ):    dict = &*_environment; break;
		case ( ORIGIN::POST ):   dict = &*_pOST;        break;
		case ( ORIGIN::GET ):    dict = &*_gET;         break;
		case ( ORIGIN::COOKIE ): dict = &*_cookies;     break;
		case ( ORIGIN::JAR ):    dict = &*_jar;     break;
		default:
			M_ASSERT( ! "bad origin" );
		}
	(*dict)[ key ] = value;
	return;
	M_EPILOG
	}

bool ORequest::lookup( HString const& key_, HString& value_, origin_t const& origin_ ) const
	{
	M_PROLOG
	ORequest::value_t value( lookup( key_, origin_ ) );
	if ( value )
		value_ = *value;
	return ( value );
	M_EPILOG
	}

ORequest::value_t ORequest::lookup( yaal::hcore::HString const& key_, origin_t const& origin_ ) const
	{
	dictionary_t::const_iterator it = _environment->find( key_ );
	bool bFound( false );
	ORequest::value_t value;
	( ! bFound ) && ( !!( origin_ & ORIGIN::ENV ) )
		&& ( bFound = ( ( it = _environment->find( key_ ) ) != _environment->end() ) )
		&& ( !! ( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::POST ) )
		&& ( bFound = ( ( it = _pOST->find( key_ ) )        != _pOST->end() ) )
		&& ( !! ( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::GET ) )
		&& ( bFound = ( ( it = _gET->find( key_ ) )         != _gET->end() ) )
		&& ( !! ( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::COOKIE ) )
		&& ( bFound = ( ( it = _cookies->find( key_ ) )     != _cookies->end() ) )
		&& ( !! ( value = it->second ) );
	return ( value );
	}

void ORequest::decompress_jar( yaal::hcore::HString const& app )
	{
	M_PROLOG
	static int const MAX_COOKIE_SIZE = 4096;
	static int const MAX_COOKIES_PER_PATH = 20;
	static int const SIZE_SIZE = 4; /* 12ab */
	HString buf;
	HString& properName = buf;
	HString jar( MAX_COOKIES_PER_PATH * MAX_COOKIE_SIZE, true );
	int cookieNo = 0;
	int size = 0;
	jar = "";
	for ( dictionary_t::iterator it = _jar->begin(); it != _jar->end(); ++ it )
		{ 
		properName.format( "%s%02d", app.raw(), cookieNo );
		if ( it->first != properName )
			continue;
		if ( ! cookieNo )
			{
			size = lexical_cast<int>( it->second.left( SIZE_SIZE ) );
			jar += it->second.mid( SIZE_SIZE );
			}
		else
			jar += it->second;
		++ cookieNo;
		}
	M_ENSURE( jar.get_length() == size );
	jar = base64::decode( jar );
	_jar->clear();
	cookieNo = 0;
	HString name;
	HTokenizer t( jar, "\001", HTokenizer::SKIP_EMPTY );
	for ( HTokenizer::HIterator it = t.begin(), endIt = t.end(); it != endIt; ++ it )
		{
		int long sepIdx = (*it).find( "=" );
		name = (*it).left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
		M_ENSURE( ! name.is_empty() );
		(*_cookies)[ name ] = (*it).mid( sepIdx + 1 ); /* + 1 for '=' char */
		}
	clog << "decompress_jar: ";
	copy( _cookies->begin(), _cookies->end(), stream_iterator( clog, " " ) );
	clog << endl;
	return;
	M_EPILOG
	}

ORequest::dictionary_ptr_t ORequest::compress_jar( yaal::hcore::HString const& app )
	{
	M_PROLOG
	static int const MAX_COOKIE_SIZE = 4096;
	static int const MAX_COOKIES_PER_PATH = 20;
	static int const META_SIZE = 512;
	static int const PAYLOAD_SIZE = MAX_COOKIE_SIZE - META_SIZE;
	HString jar( MAX_COOKIES_PER_PATH * MAX_COOKIE_SIZE, true );
	jar = "";
	int cookieNo = 0;
	clog << "compress_jar: ";
	copy( _cookies->begin(), _cookies->end(), stream_iterator( clog, " " ) );
	clog << endl;
	for ( dictionary_t::const_iterator it = _cookies->begin(), endIt( _cookies->end() ); it != endIt; ++ it, ++ cookieNo )
		{
		if ( cookieNo )
			jar += "\001";
		jar += it->first;
		jar += '=';
		jar += it->second;
		}
	jar = base64::encode( jar );
	int size = static_cast<int>( jar.get_length() );
	HString properName;
	HString payload;
	cookieNo = 0;
	_jar->clear();
	for ( int offset = 0; offset < size; offset += PAYLOAD_SIZE, ++ cookieNo )
		{
		properName.format( "%s%02d", app.raw(), cookieNo );
		if ( ! offset )
			{
			payload.format( "%04d", size );
			payload += jar.mid( offset, PAYLOAD_SIZE );
			}
		else
			payload = jar.mid( offset, PAYLOAD_SIZE );
		(*_jar)[ properName ] = payload;
		}
	return ( _jar );
	M_EPILOG
	}

HSocket::ptr_t ORequest::socket( void )
	{
	return ( _socket );
	}

HSocket::ptr_t const ORequest::socket( void ) const
	{
	return ( _socket );
	}

ORequest::const_iterator ORequest::begin( void ) const
	{
	dictionary_t::const_iterator it = _gET->begin();
	origin_t o = it != _gET->end() ? ORIGIN::GET : ORIGIN::POST;
	return ( const_iterator( this, o, o == ORIGIN::GET ? it : _pOST->begin() ) );
	}

ORequest::const_iterator ORequest::end( void ) const
	{
	return ( const_iterator( this, ORIGIN::POST, _pOST->end() ) );
	}


ORequest::HConstIterator::HConstIterator( HConstIterator const& it )
	: _owner( it._owner ), _origin( it._origin ), _it( it._it )
	{
	}

bool ORequest::HConstIterator::operator != ( HConstIterator const& it ) const
	{
	return ( ( _origin != it._origin ) || ( _it != it._it ) );
	}

ORequest::HConstIterator& ORequest::HConstIterator::operator ++ ( void )
	{
	M_ASSERT( ( _origin == ORequest::ORIGIN::GET ) || ( _it != _owner->_pOST->end() ) );
	++ _it;
	if ( ( _origin == ORequest::ORIGIN::GET ) && ( _it == _owner->_gET->end() ) )
		{
		_origin = ORequest::ORIGIN::POST;
		_it = _owner->_pOST->begin();
		}
	return ( *this );
	}

ORequest::dictionary_t::value_type const& ORequest::HConstIterator::operator* ( void ) const
	{
	return ( *_it );
	}

ORequest::HConstIterator::HConstIterator( ORequest const* owner_,
		ORequest::origin_t const& origin, ORequest::dictionary_t::const_iterator it )
	: _owner( owner_ ), _origin( origin ), _it( it )
	{
	}

}

