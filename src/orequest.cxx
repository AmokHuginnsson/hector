/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	orequest.cxx - this file is integral part of `hector' project.

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

#include <cstring>

#include <yaal/hcore/htokenizer.hxx>
#include <yaal/hcore/hfile.hxx>
#include <yaal/hcore/hformat.hxx>
#include <yaal/hcore/hregex.hxx>
#include <yaal/tools/base64.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "orequest.hxx"
#include "http.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector {

ORequest::origin_t const ORequest::ORIGIN::NONE = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::ENV = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::COOKIE = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::GET = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::POST = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::JAR = ORequest::origin_t::new_flag();
ORequest::origin_t const ORequest::ORIGIN::ANY =
	ORequest::ORIGIN::ENV | ORequest::ORIGIN::COOKIE
	| ORequest::ORIGIN::GET | ORequest::ORIGIN::POST | ORequest::ORIGIN::JAR;

ORequest::ORequest( HStreamInterface::ptr_t socket_ )
	: _socket( socket_ )
	, _environment( new dictionary_t() )
	, _get( new dictionary_t() )
	, _post( new dictionary_t() )
	, _cookies( new dictionary_t() )
	, _jar( new dictionary_t() )
	, _messages() {
}

void ORequest::update( HString const& key, HString const& value, origin_t const& origin ) {
	M_PROLOG
	M_ASSERT( ORIGIN::NONE.index()   == 0 );
	M_ASSERT( ORIGIN::ENV.index()    == 1 );
	M_ASSERT( ORIGIN::COOKIE.index() == 2 );
	M_ASSERT( ORIGIN::GET.index()    == 3 );
	M_ASSERT( ORIGIN::POST.index()   == 4 );
	M_ASSERT( ORIGIN::JAR.index()    == 5 );
	M_ASSERT( ORIGIN::ANY.value()    == 31 );

	dictionary_t* dict[] = {
		NULL, /* so origin_t::index() maps directly to this array */
		&*_environment,
		&*_cookies,
		&*_get,
		&*_post,
		&*_jar
	};
	/*
	 * HBitFlag::index() contains proper assertion check.
	 */
	M_ASSERT( ( origin.index() >= 1 ) && ( origin.index() < countof ( dict ) ) );
	( *( dict[origin.index()] ) )[ key ] = value;
	return;
	M_EPILOG
}

bool ORequest::lookup( HString const& key_, HString& value_, origin_t const& origin_ ) const {
	M_PROLOG
	ORequest::value_t value( lookup( key_, origin_ ) );
	if ( value )
		value_ = *value;
	return ( ! value );
	M_EPILOG
}

ORequest::value_t ORequest::lookup( yaal::hcore::HString const& key_, origin_t const& origin_ ) const {
	M_PROLOG
	dictionary_t::const_iterator it = _environment->find( key_ );
	bool bFound( false );
	ORequest::value_t value;
	( ! bFound ) && ( !!( origin_ & ORIGIN::ENV ) )
		&& ( bFound = ( ( it = _environment->find( key_ ) ) != _environment->end() ) )
		&& ( !! *( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::POST ) )
		&& ( bFound = ( ( it = _post->find( key_ ) )        != _post->end() ) )
		&& ( !! *( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::GET ) )
		&& ( bFound = ( ( it = _get->find( key_ ) )         != _get->end() ) )
		&& ( !! *( value = it->second ) );
	( ! bFound ) && ( !!( origin_ & ORIGIN::COOKIE ) )
		&& ( bFound = ( ( it = _cookies->find( key_ ) )     != _cookies->end() ) )
		&& ( !! *( value = it->second ) );
	OUT << "key: " << key_ << ", value: " << ( value ? *value : HString( "(nil)" ) ) << endl;
	return ( value );
	M_EPILOG
}

bool ORequest::is_ssl( void ) const {
	M_PROLOG
	bool ssl( false );
	ORequest::value_t https( lookup( HTTP::HTTPS, ORequest::ORIGIN::ENV ) );
	if ( ! https ) {
		ORequest::value_t httpHost( lookup( HTTP::HTTP_HOST, ORequest::ORIGIN::ENV ) );
		ORequest::value_t serverPort( lookup( HTTP::SERVER_PORT, ORequest::ORIGIN::ENV ) );
		if ( httpHost && serverPort ) {
			int long pos( httpHost->find( ':' ) );
			ssl = ( pos != HString::npos ) && ( pos < ( httpHost->get_length() - 1 ) ) && ( ::strcmp( serverPort->raw(), httpHost->raw() + pos + 1 ) );
		}
	} else
		ssl = ( *https == "on" );
	return ( ssl );
	M_EPILOG
}

bool ORequest::is_mobile( void ) const {
	static HRegex const b(
			"(android|bb[[:digit:]]+|meego).+mobile"
			"|avantgo|bada/"
			"|blackberry"
			"|blazer"
			"|compal|elaine"
			"|fennec"
			"|hiptop"
			"|iemobile"
			"|ip(hone|od)"
			"|iris|kindle"
			"|lge "
			"|maemo"
			"|midp"
			"|mmp"
			"|mozilla/[[:digit:]\\.]+ \\(mobile;"
			"|netfront"
			"|opera m(ob|in)i"
			"|palm( os)?"
			"|phone"
			"|p(ixi|re)/"
			"|plucker"
			"|pocket"
			"|psp"
			"|series(4|6)0"
			"|symbian"
			"|treo"
			"|up\\.(browser|link)"
			"|vodafone"
			"|wap"
			"|windows (ce|phone)"
			"|xda"
			"|xiino",
		HRegex::compile_t( HRegex::COMPILE::IGNORE_CASE ) | HRegex::COMPILE::EXTENDED );
	static HRegex const v(
			"1207" "|6310" "|6590" "|3gso" "|4thp" "|50[1-6]i" "|770s" "|802s"
			"|a wa" "|abac" "|ac(er|oo|s-)" "|ai(ko|rn)" "|al(av|ca|co)" "|amoi"
			"|an(ex|ny|yw)" "|aptu" "|ar(ch|go)" "|as(te|us)" "|attw" "|au(di|-m|r |s )"
			"|avan" "|be(ck|ll|nq)" "|bi(lb|rd)" "|bl(ac|az)" "|br(e|v)w" "|bumb"
			"|bw-(n|u)" "|c55/" "|capi" "|ccwa" "|cdm-" "|cell" "|chtm" "|cldc"
			"|cmd-" "|co(mp|nd)" "|craw|da(it|ll|ng)" "|dbte" "|dc-s" "|devi"
			"|dica" "|dmob" "|do(c|p)o" "|ds(12|-d)" "|el(49|ai)" "|em(l2|ul)"
			"|er(ic|k0)" "|esl8" "|ez([4-7]0|os|wa|ze)" "|fetc|fly(-|_)" "|g1 u"
			"|g560" "|gene" "|gf-5" "|g-mo" "|go(.w|od)" "|gr(ad|un)" "|haie"
			"|hcit" "|hd-(m|p|t)" "|hei-" "|hi(pt|ta)" "|hp( i|ip)" "|hs-c"
			"|ht(c(-| |_|a|g|p|s|t)|tp)" "|hu(aw|tc)|i-(20|go|ma)|i230|iac( |-|/)"
			"|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu"
			"|jigs|kddi|keji|kgt( |/)|klon|kpt |kwc-|kyo(c|k)|le(no|xi)|"
			"lg( g|/(k|l|u)|50|54|-[a-w])|libw|lynx|m1-w|m3ga|m50/"
			"|ma(te|ui|xo)|mc(01|21|ca)|m-cr|me(rc|ri)|mi(o8|oa|ts)"
			"|mmef|mo(01|02|bi|de|do|t(-| |o|v)|zz)|mt(50|p1|v )"
			"|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)"
			"|ne((c|m)-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)"
			"|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|-([1-8]|c))|phil"
			"|pire|pl(ay|uc)|pn-2|po(ck|rt|se)|prox|psio|pt-g|qa-a"
			"|qc(07|12|21|32|60|-[2-7]|i-)|qtek|r380|r600|raks|rim9"
			"|ro(ve|zo)|s55/|sa(ge|ma|mm|ms|ny|va)|sc(01|h-|oo|p-)|sdk/"
			"|se(c(-|0|1)|47|mc|nd|ri)|sgh-|shar|sie(-|m)|sk-0|sl(45|id)"
			"|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h-|v-|v )|sy(01|mb)"
			"|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl-|tdg-|tel(i|m)|tim-"
			"|t-mo|to(pl|sh)|ts(70|m-|m3|m5)|tx-9|up(.b|g1|si)|utst"
			"|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|-v)|vm40|voda|vulc"
			"|vx(52|53|60|61|70|80|81|83|85|98)|w3c(-| )|webc|whit"
			"|wi(g |nc|nw)|wmlb|wonu|x700|yas-|your|zeto|zte-",
		HRegex::compile_t( HRegex::COMPILE::IGNORE_CASE ) | HRegex::COMPILE::EXTENDED );
	ORequest::value_t ua( lookup( HTTP::HTTP_USER_AGENT, ORequest::ORIGIN::ENV ) );
	return ( b.matches( *ua ) || v.matches( ua->substr( 0, 4 ) ) );
}

void ORequest::decompress_jar( yaal::hcore::HString const& app ) {
	M_PROLOG
	static int const MAX_COOKIE_SIZE = 4096;
	static int const MAX_COOKIES_PER_PATH = 20;
	static int const SIZE_SIZE = 4; /* 12ab */
	HString buf;
	HString& properName = buf;
	HString jar( MAX_COOKIES_PER_PATH * MAX_COOKIE_SIZE, true );
	int cookieNo( 0 );
	int size( 0 );
	jar.clear();
	for ( dictionary_t::const_iterator it( _jar->begin() ), endIt( _jar->end() ); it != endIt; ++ it ) {
		properName.format( "%s%02d", app.raw(), cookieNo );
		if ( it->first != properName )
			continue;
		if ( ! cookieNo ) {
			size = lexical_cast<int>( it->second.left( SIZE_SIZE ).trim_left( "0" ) );
			jar += it->second.mid( SIZE_SIZE );
		} else
			jar += it->second;
		++ cookieNo;
	}
	M_ENSURE( jar.get_length() == size, lexical_cast<HString>( HFormat( "%ld != %d" ) % jar.get_length() % size ) );
	jar = base64::decode( jar );
	_jar->clear();
	cookieNo = 0;
	HString name;
	HTokenizer t( jar, "\001", HTokenizer::SKIP_EMPTY );
	for ( HTokenizer::HIterator it( t.begin() ), endIt( t.end() ); it != endIt; ++ it ) {
		int long sepIdx = (*it).find( "=" );
		name = (*it).left( sepIdx >= 0 ? sepIdx : meta::max_signed<int long>::value );
		M_ENSURE( ! name.is_empty() );
		(*_cookies)[ name ] = (*it).mid( sepIdx + 1 ); /* + 1 for '=' char */
	}
	return;
	M_EPILOG
}

ORequest::dictionary_ptr_t ORequest::compress_jar( yaal::hcore::HString const& app ) {
	M_PROLOG
	static int const MAX_COOKIE_SIZE = 4096;
	static int const MAX_COOKIES_PER_PATH = 20;
	static int const META_SIZE = 512;
	static int const PAYLOAD_SIZE = MAX_COOKIE_SIZE - META_SIZE;
	HString jar( MAX_COOKIES_PER_PATH * MAX_COOKIE_SIZE, true );
	jar = "";
	int cookieNo = 0;
	for ( dictionary_t::const_iterator it = _cookies->begin(), endIt( _cookies->end() ); it != endIt; ++ it, ++ cookieNo ) {
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
	for ( int offset = 0; offset < size; offset += PAYLOAD_SIZE, ++ cookieNo ) {
		properName.format( "%s%02d", app.raw(), cookieNo );
		if ( ! offset ) {
			payload.format( "%04d", size );
			payload += jar.mid( offset, PAYLOAD_SIZE );
		} else
			payload = jar.mid( offset, PAYLOAD_SIZE );
		(*_jar)[ properName ] = payload;
	}
	return ( _jar );
	M_EPILOG
}

void ORequest::message( yaal::hcore::HString const& id_, yaal::hcore::LOG_LEVEL::priority_t type_, yaal::hcore::HString const& data_ ) {
	M_PROLOG
	_messages[id_].emplace_back( type_, data_ );
	return;
	M_EPILOG
}

ORequest::message_map_t const& ORequest::messages( void ) const {
	return ( _messages );
}

HStreamInterface::ptr_t ORequest::socket( void ) {
	return ( _socket );
}

HStreamInterface::ptr_t const ORequest::socket( void ) const {
	return ( _socket );
}

ORequest::const_iterator ORequest::begin( void ) const {
	dictionary_t::const_iterator it = _get->begin();
	origin_t o = it != _get->end() ? ORIGIN::GET : ORIGIN::POST;
	return ( const_iterator( this, o, o == ORIGIN::GET ? it : _post->begin() ) );
}

ORequest::const_iterator ORequest::end( void ) const {
	return ( const_iterator( this, ORIGIN::POST, _post->end() ) );
}


ORequest::HConstIterator::HConstIterator( HConstIterator const& it )
	: _owner( it._owner ), _origin( it._origin ), _it( it._it ) {
}

bool ORequest::HConstIterator::operator != ( HConstIterator const& it ) const {
	return ( ( _origin != it._origin ) || ( _it != it._it ) );
}

ORequest::HConstIterator& ORequest::HConstIterator::operator ++ ( void ) {
	M_ASSERT( ( _origin == ORequest::ORIGIN::GET ) || ( _it != _owner->_post->end() ) );
	++ _it;
	if ( ( _origin == ORequest::ORIGIN::GET ) && ( _it == _owner->_get->end() ) ) {
		_origin = ORequest::ORIGIN::POST;
		_it = _owner->_post->begin();
	}
	return ( *this );
}

ORequest::dictionary_t::value_type const& ORequest::HConstIterator::operator* ( void ) const {
	return ( *_it );
}

ORequest::HConstIterator::HConstIterator( ORequest const* owner_,
		ORequest::origin_t const& origin, ORequest::dictionary_t::const_iterator it )
	: _owner( owner_ ), _origin( origin ), _it( it ) {
}

}

