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
	HString action;
	if ( ! req.lookup( "action", action ) && ( action == "ajax" ) )
		{
		app.f_oDOM.clear();
		app.f_oDOM.create_root( "xml" );
		HXml::HNodeProxy root = app.f_oDOM.get_root();
		root.add_node( HXml::HNode::TYPE::D_CONTENT, "Hello world!, query: " );
		HString query;
		if ( ! req.lookup( "query", query ) )
			(*root.add_node( HXml::HNode::TYPE::D_NODE, "b" )).add_node( HXml::HNode::TYPE::D_CONTENT, query );;
		}
	else
		{
		HString pageName;
		if ( req.lookup( "page", pageName ) )
			pageName = "info";
		HXml::HNodeProxy page = app.f_oDOM.get_element_by_id( "page" );
		HXml waste;
		waste.create_root( "x" );
		HXml::HNodeProxy dummy = waste.get_root();
		for ( HXml::HIterator it = page.begin(); it != page.end(); )
			{
			HXml::HNode::properties_t::iterator classIt = (*it).properties().find( "class" );
			if ( ( classIt != (*it).properties().end() ) && ( classIt->second == "content" ) )
				{
				HXml::HNode::properties_t::iterator idIt = (*it).properties().find( "id" );
				if ( ( idIt != (*it).properties().end() ) && ( idIt->second != pageName ) )
					{
					HXml::HIterator del = it;
					++ it;
					dummy.move_node( *del );
					}
				else
					++ it;
				}
			else
				++ it;
			}
		}
	}

}

}

