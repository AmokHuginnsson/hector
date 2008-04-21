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
using namespace yaal::hcore;
using namespace yaal::tools;

namespace hector
{

namespace cgi
{

bool is_kind_of( yaal::tools::HXml::HNodeProxy const& node, char const* const kind )
	{
	M_PROLOG
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
	M_EPILOG
	}

void build_keep_db( HXml::HNodeProxy keep, ORequest const& req, keep_t& db )
	{
	M_PROLOG
	static char const* const D_NODE_KEEP_RULE = "rule";
	static char const* const D_ATTRIBUTE_KIND = "kind";
	static char const* const D_ATTRIBUTE_DEFAULT = "default";
	for ( HXml::HIterator it = keep.begin(); it != keep.end(); ++ it )
		{
		M_ENSURE( (*it).get_type() == HXml::HNode::TYPE::D_NODE );
		M_ENSURE( (*it).get_name() == D_NODE_KEEP_RULE );
		HXml::HNode::properties_t& props = (*it).properties();
		HXml::HNode::properties_t::iterator kind = props.find( D_ATTRIBUTE_KIND );
		M_ENSURE( kind != props.end() );
		HString keepItem( "" );
		if ( req.lookup( kind->second, keepItem ) )
			{
			HXml::HNode::properties_t::iterator defaultIt = props.find( D_ATTRIBUTE_DEFAULT );
			if ( defaultIt != props.end() )
				keepItem = defaultIt->second;
			}
		if ( ! keepItem.is_empty() )
			db.insert( keepItem );
		}
	return;
	M_EPILOG
	}

void waste_children( yaal::tools::HXml::HNodeProxy node,
		ORequest const& req, HXml::HNodeProxy* selfwaste )
	{
	M_PROLOG
	static char const* const D_ATTRIBUTE_ID = "id";
	static char const* const D_CLASS_WASTEABLE = "wasteable";
	static char const* const D_NODE_KEEP = "keep";
	static HXml waste;
	if ( ! selfwaste )
		{
		waste.create_root( "x" );
		HXml::HNodeProxy root = waste.get_root();
		selfwaste = &root;
		}
	keep_t keep;
	for ( HXml::HIterator it = node.begin(); it != node.end(); )
		{
		HXml::HIterator del = it;
		++ it;
		if ( (*del).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( (*del).get_name() == D_NODE_KEEP )
				{
				build_keep_db( *del, req, keep );
				selfwaste->move_node( *del );
				}
			else if ( is_kind_of( *del, D_CLASS_WASTEABLE ) )
				{
				HXml::HNode::properties_t::iterator idIt = (*del).properties().find( D_ATTRIBUTE_ID );
				if ( ( idIt != (*del).properties().end() ) && ( keep.find( idIt->second ) == keep.end() ) )
					selfwaste->move_node( *del );
				else
					waste_children( *del, req, selfwaste );
				}
			else
				waste_children( *del, req, selfwaste );
			}
		}
	M_EPILOG
	}

}

}

