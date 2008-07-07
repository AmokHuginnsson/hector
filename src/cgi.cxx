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
#include "setup.h"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector
{

namespace cgi
{

static char const* const D_ATTRIBUTE_ID = "id";
static char const* const D_ATTRIBUTE_CLASS = "class";

bool is_kind_of( yaal::tools::HXml::HNodeProxy const& node, char const* const kind )
	{
	M_PROLOG
	static char const* const D_CLASS_SEPARATOR = " \t";
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

void build_keep_db( HXml::HNodeProxy keep, ORequest const& req, keep_t& db, default_t& defaults )
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
				{
				keepItem = defaultIt->second;
				defaults.insert( kind->second, keepItem );
				}
			}
		if ( ! keepItem.is_empty() )
			db.insert( keepItem );
		}
	return;
	M_EPILOG
	}

void waste_children( yaal::tools::HXml::HNodeProxy node,
		ORequest const& req, default_t& defaults,
		HXml::HNodeProxy* selfwaste )
	{
	M_PROLOG
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
				build_keep_db( *del, req, keep, defaults );
				selfwaste->move_node( *del );
				}
			else if ( is_kind_of( *del, D_CLASS_WASTEABLE ) )
				{
				HXml::HNode::properties_t::iterator idIt = (*del).properties().find( D_ATTRIBUTE_ID );
				if ( ( idIt != (*del).properties().end() ) && ( keep.find( idIt->second ) == keep.end() ) )
					selfwaste->move_node( *del );
				else
					waste_children( *del, req, defaults, selfwaste );
				}
			else
				waste_children( *del, req, defaults, selfwaste );
			}
		}
	return;
	M_EPILOG
	}

void mark_children( yaal::tools::HXml::HNodeProxy node,
		ORequest const& req, default_t const& defaults, HXml& doc )
	{
	M_PROLOG
	static char const* const D_CLASS_MARKABLE = "markable";
	static char const* const D_CLASS_CURRENT = " current";
	for ( HXml::HIterator it = node.begin(); it != node.end(); ++ it )
		{
		if ( (*it).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( is_kind_of( *it, D_CLASS_MARKABLE ) )
				{
				HXml::HNode::properties_t& props = (*it).properties();
				HXml::HNode::properties_t::iterator id = props.find( D_ATTRIBUTE_ID );
				if ( id != props.end() )
					{
					HString subject = id->second.split( "-", 1 );
					if ( ! subject.is_empty() )
						{
						HString object;
						if ( req.lookup( subject, object ) )
							{
							default_t::const_iterator defaultIt = defaults.find( subject );
							if ( defaultIt != defaults.end() )
								object = defaultIt->second;
							}
						if ( ! object.is_empty() )
							{
							HXml::HNodeProxy mark = doc.get_element_by_id( subject + "-" + object );
							if ( !! mark )
								mark.properties()[ D_ATTRIBUTE_CLASS ] += D_CLASS_CURRENT;
							}
						}
					}
				}
			else
				mark_children( *it, req, defaults, doc );
			}
		}
	return;
	M_EPILOG
	}

void move_children( yaal::tools::HXml::HNodeProxy node, ORequest const& req,
		yaal::tools::HXml& doc, HXml::HNodeProxy* selfwaste )
	{
	M_PROLOG
	static char const* const D_NODE_MOVE = "move";
	static char const* const D_ATTRIBUTE_TO = "to";
	static HXml waste;
	if ( ! selfwaste )
		{
		waste.create_root( "x" );
		HXml::HNodeProxy root = waste.get_root();
		selfwaste = &root;
		}
	for ( HXml::HIterator it = node.begin(); it != node.end(); )
		{
		HXml::HIterator del = it;
		++ it;
		if ( (*del).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( (*del).get_name() == D_NODE_MOVE )
				{
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator toIt = props.find( D_ATTRIBUTE_TO );
				if ( toIt != props.end() )
					{
					HXml::HNodeProxy to = doc.get_element_by_path( toIt->second );
					if ( !! to )
						{
						for ( HXml::HIterator moveIt = (*del).begin(); moveIt != (*del).end(); )
							{
							HXml::HIterator move = moveIt;
							++ moveIt;
							to.move_node( *move );
							}
						}
					}
				selfwaste->move_node( *del );
				}
			else
				move_children( *del, req, doc );
			}
		}
	return;
	M_EPILOG
	}

void run_query( yaal::tools::HXml::HNodeProxy node, HDataBase::ptr_t db, yaal::tools::HXml& doc, yaal::tools::HXml::HNodeProxy* pick )
	{
	M_PROLOG
	static char const* const D_NODE_QUERY = "query";
//	static char const* const D_NODE_ITEM = "item";
	static char const* const D_ATTRIBUTE_SQL = "sql";
	static HXml waste;
	if ( ! pick )
		{
		waste.create_root( "x" );
		HXml::HNodeProxy root = waste.get_root();
		pick = &root;
		}
	for ( HXml::HIterator child = node.begin(); child != node.end(); )
		{
		HXml::HIterator del = child;
		++ child;
		if ( (*del).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( (*del).get_name() == D_NODE_QUERY )
				{
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator sqlIt = props.find( D_ATTRIBUTE_SQL );
				if ( sqlIt != props.end() )
					{
					HString sql = sqlIt->second;
					if ( ! sql.is_empty() )
						{
						HRecordSet::ptr_t rs = db->query( sql );
						for ( HRecordSet::iterator it = rs->begin(); it != rs->end(); ++ it )
							cout << it[ 0 ] << "|" << it[ 1 ] << endl;
						}
					}
				pick->move_node( *del );
				}
			else
				run_query( *del, db, doc, pick );
			}
		}
	return;
	M_EPILOG
	}

}

}

