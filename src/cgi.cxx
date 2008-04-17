/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	cgi.cxx - this file is integral part of `hector' project.

	i.  You may not make any changes in Copyright information.
	ii. You must attach Copyright information to any part of every copy
	    of this software.

Copyright:

 You are free to use this program as is, you can redistribute binary
 package freely but:
  1. You can not use any part of sources of this software.
  2. You can not redistribute any part of sources of this software.
  3. No reverse engineering is allowed.
  4. If you want redistribute binary package you can not demand any fees
     for this software.
     You can not even demand cost of the carrier (CD for example).
  5. You can not include it to any commercial enterprise (for example 
     as a free add-on to payed software or payed newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#include <iostream>

#include <yaal/yaal.h>
M_VCSID( "$Id$")
#include "cgi.h"

using namespace std;
using namespace yaal::tools;

namespace hector
{

namespace cgi
{

bool is_kind_of( yaal::tools::HXml::HNodeProxy const& node, char const* const kind )
	{
	static char const* const D_CLASS_SEPARATOR = " \t";
	static char const* const D_ATTRIBUTE_CLASS = "class";
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::D_NODE );
	HXml::HNode::properties_t const& props = node.properties();
	HXml::HNode::properties_t::const_iterator classIt = props.find( D_ATTRIBUTE_CLASS );
	bool is = false;
	if ( classIt != props.end() )
		{
		int idx = 0;
		bool leftLimit = ( ( idx = classIt->second.find( kind ) ) >= 0 )
			&& ( ! idx || strchr( D_CLASS_SEPARATOR, classIt->second[ idx - 1 ] ) );
		if ( leftLimit )
			{
			int len = ::strlen( kind );
			is = ( ( idx + len ) >= classIt->second.get_length() ) || ::strchr( D_CLASS_SEPARATOR, classIt->second[ idx + len ] );
			}
		}
	return ( is );
	}

void waste_children( yaal::tools::HXml::HNodeProxy node, keep_t const& keep )
	{
	static char const* const D_ATTRIBUTE_ID = "id";
	char const* const D_CLASS_WASTEABLE = "wasteable";
	HXml waste;
	waste.create_root( "x" );
	HXml::HNodeProxy dummy = waste.get_root();
	for ( HXml::HIterator it = node.begin(); it != node.end(); )
		{
		if ( (*it).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( is_kind_of( *it, D_CLASS_WASTEABLE ) )
				{
				HXml::HNode::properties_t::iterator idIt = (*it).properties().find( D_ATTRIBUTE_ID );
				if ( ( idIt != (*it).properties().end() ) && ( keep.find( idIt->second ) == keep.end() ) )
					{
					HXml::HIterator del = it;
					++ it;
					dummy.move_node( *del );
					}
				else
					++ it;
				}
			else
				{
				waste_children( *it, keep );
				++ it;
				}
			}
		else
			++ it;
		}
	}

}

}

