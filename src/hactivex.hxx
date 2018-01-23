/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef HECTOR_OACTIVEX_HXX_INCLUDED
#define HECTOR_OACTIVEX_HXX_INCLUDED

#include "application.hxx"

namespace hector {

struct HSession;
class HApplicationServer;

class HActiveX {
	yaal::hcore::HString _binaryPath;
	yaal::tools::HPlugin::ptr_t _activeX;
	HApplication::ptr_t _application;
public:
	HActiveX( yaal::hcore::HString const& path = yaal::hcore::HString() )
		: _binaryPath( path )
		, _activeX()
		, _application() {
	}
	static HActiveX get_instance(
		HApplicationServer*,
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		yaal::hcore::HString const&,
		HApplication::MODE
	);
	void reload_binary( void );
	void handle_logic( ORequest&, HSession& );
	void generate_page( ORequest const&, HSession const& );
	HApplication const& app( void ) const;
	HApplication::sessions_t& sessions( void );
};

typedef yaal::hcore::HExceptionT<HActiveX> HActiveXException;

}

#endif /* not HECTOR_OACTIVEX_HXX_INCLUDED */

