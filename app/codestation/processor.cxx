#include <yaal/yaal.h>

extern "C"
{
#include "src/application.h"
}

using namespace yaal::hcore;
using namespace yaal::tools;

extern "C"
{

namespace hector
{

void application_processor( HApplication& app, ORequest& req )
	{
	HString pageName;
	if ( req.lookup( "page", pageName ) )
		pageName = "info";
	HXml::HNodeProxy page = app.f_oDOM.get_element_by_id( "page" );
	for ( HXml::HIterator it = page.begin(); it != page.end(); )
		{
		HXml::HNode::properties_t::iterator classIt = (*it).properties().find( "class" );
		if ( ( classIt != (*it).properties().end() ) && ( classIt->second == "content" ) )
			{
			HXml::HNode::properties_t::iterator idIt = (*it).properties().find( "id" );
			if ( ( idIt != (*it).properties().end() ) && ( idIt->second != pageName ) )
				it = page.remove_node( it );
			else
				++ it;
			}
		else
			++ it;
		}
	}

}

}

