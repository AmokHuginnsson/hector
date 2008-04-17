#include <yaal/yaal.h>

#include "src/cgi.h"

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
		cgi::keep_t keep;
		keep.insert( pageName );
		cgi::waste_children( app.f_oDOM.get_root(), keep );
		}
	}

}

}

