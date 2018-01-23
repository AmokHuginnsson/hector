/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_OREQUEST_HXX_INCLUDED
#define HECTOR_OREQUEST_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hpointer.hxx>
#include <yaal/hcore/hbitflag.hxx>
#include <yaal/hcore/hmap.hxx>
#include <yaal/hcore/hsocket.hxx>
#include <yaal/hcore/hlog.hxx>
#include <yaal/tools/hoptional.hxx>

namespace hector {

class ORequest {
	typedef ORequest this_type;
public:
	typedef yaal::hcore::HMap<yaal::hcore::HString, yaal::hcore::HString> dictionary_t;
	typedef yaal::hcore::HPointer<dictionary_t> dictionary_ptr_t;
	struct ORIGIN;
	typedef yaal::hcore::HBitFlag<ORIGIN> origin_t;
	struct ORIGIN {
		static origin_t const NONE;
		static origin_t const ENV;
		static origin_t const COOKIE;
		static origin_t const GET;
		static origin_t const POST;
		static origin_t const JAR;
		static origin_t const ANY;
	};
	struct OMessage {
		yaal::hcore::LOG_LEVEL::priority_t _type;
		yaal::hcore::HString _data;
		OMessage( yaal::hcore::LOG_LEVEL::priority_t type_, yaal::hcore::HString const& data_ )
			: _type( type_ )
			, _data( data_ ) {
			return;
		}
	};
	typedef yaal::hcore::HArray<OMessage> messages_t;
	typedef yaal::hcore::HHashMap<yaal::hcore::HString, messages_t> message_map_t;
	class HConstIterator;
	typedef HConstIterator const_iterator;
	typedef yaal::tools::HOptional<yaal::hcore::HString const&> value_t;
private:
	yaal::hcore::HStreamInterface::ptr_t _socket;
	dictionary_ptr_t _environment;
	dictionary_ptr_t _get;
	dictionary_ptr_t _post;
	dictionary_ptr_t _cookies;
	dictionary_ptr_t _jar;
	message_map_t _messages;
public:
	ORequest( yaal::hcore::HStreamInterface::ptr_t = yaal::hcore::HStreamInterface::ptr_t() );
	ORequest( ORequest&& ) = default;
	void update( yaal::hcore::HString const&, yaal::hcore::HString const&, origin_t const& );
	bool lookup( yaal::hcore::HString const&, yaal::hcore::HString&, origin_t const& = ORIGIN::ANY ) const;
	value_t lookup( yaal::hcore::HString const&, origin_t const& = ORIGIN::ANY ) const;
	void decompress_jar( yaal::hcore::HString const& );
	dictionary_ptr_t compress_jar( yaal::hcore::HString const& );
	bool is_ssl( void ) const;
	bool is_mobile( void ) const;
	yaal::hcore::HStreamInterface::ptr_t socket( void );
	yaal::hcore::HStreamInterface::ptr_t const socket( void ) const;
	const_iterator begin( void ) const;
	const_iterator end( void ) const;
	void message( yaal::hcore::HString const&, yaal::hcore::LOG_LEVEL::priority_t, yaal::hcore::HString const& );
	message_map_t const& messages( void ) const;
private:
	ORequest( ORequest const& ) = delete;
	ORequest& operator = ( ORequest const& ) = delete;
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

