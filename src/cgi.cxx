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

#include <yaal/yaal.hxx>
M_VCSID( "$Id: "__ID__" $" )
#include "cgi.hxx"
#include "setup.hxx"

using namespace std;
using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector
{

namespace cgi
{

static HString const D_ATTRIBUTE_CLASS( "class" );

bool is_in_attribute( yaal::tools::HXml::HNode::properties_t const& props, HString const& attribute, HString const& element )
	{
	M_PROLOG
	static char const* const D_ELEMENT_SEPARATOR = " \t";
	HXml::HNode::properties_t::const_iterator attrIt = props.find( attribute );
	bool is = false;
	if ( attrIt != props.end() )
		{
		int idx = 0;
		bool leftLimit = ( ( idx = static_cast<int>( attrIt->second.find( element ) ) ) >= 0 )
			&& ( ! idx || ::strchr( D_ELEMENT_SEPARATOR, attrIt->second[ idx - 1 ] ) );
		if ( leftLimit )
			{
			int len = static_cast<int>( element.get_length() );
			is = ( ( idx + len ) >= attrIt->second.get_length() ) || ::strchr( D_ELEMENT_SEPARATOR, attrIt->second[ idx + len ] );
			}
		}
	return ( is );
	M_EPILOG
	}

bool is_kind_of( yaal::tools::HXml::HNode::properties_t const& props, HString const& kind )
	{
	M_PROLOG
	return ( is_in_attribute( props, D_ATTRIBUTE_CLASS, kind ) );
	M_EPILOG
	}

bool has_attribute( yaal::tools::HXml::HNode::properties_t const& props, HString const& attribute )
	{
	M_PROLOG
	HXml::HNode::properties_t::const_iterator attrIt = props.find( attribute );
	return ( attrIt != props.end() );
	M_EPILOG
	}

bool is_in_attribute( yaal::tools::HXml::HNodeProxy const& node, HString const& attribute, HString const& element )
	{
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::D_NODE );
	HXml::HNode::properties_t const& props = node.properties();
	return ( is_in_attribute( props, attribute, element ) );
	M_EPILOG
	}

bool is_kind_of( yaal::tools::HXml::HNodeProxy const& node, HString const& kind )
	{
	M_PROLOG
	return ( is_in_attribute( node, D_ATTRIBUTE_CLASS, kind ) );
	M_EPILOG
	}

bool has_attribute( yaal::tools::HXml::HNodeProxy const& node, HString const& attribute )
	{
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::D_NODE );
	HXml::HNode::properties_t const& props = node.properties();
	return ( has_attribute( props, attribute ) );
	M_EPILOG
	}

void build_keep_db( HXml::HNodeProxy keep, ORequest const& req, keep_t& db, keep_t& global, default_t& defaults )
	{
	M_PROLOG
	static HString const D_NODE_KEEP_RULE( "rule" );
	static HString const D_ATTRIBUTE_KIND( "kind" );
	static HString const D_ATTRIBUTE_DEFAULT( "default" );
	static HString const D_ATTRIBUTE_VALID( "valid" );
	static HString const D_ATTRIBUTE_GLOBAL( "global" );
	/*
	 * Look thru all <rule />'s.
	 */
	for ( HXml::HIterator it = keep.begin(); it != keep.end(); ++ it )
		{
		M_ENSURE( (*it).get_type() == HXml::HNode::TYPE::D_NODE );
		M_ENSURE( (*it).get_name() == D_NODE_KEEP_RULE );
		HXml::HNode::properties_t& props = (*it).properties();
		HXml::HNode::properties_t::iterator kind = props.find( D_ATTRIBUTE_KIND );
		/*
		 * All <rule />'s must have kind="" attribute.
		 */
		M_ENSURE( kind != props.end() );
		HString keepItem( "" );
		/*
		 * Let's find out if request have that `kind' specified.
		 */
		if ( req.lookup( kind->second, keepItem )
				|| ( has_attribute( props, D_ATTRIBUTE_VALID )
					&& ! is_in_attribute( props, D_ATTRIBUTE_VALID, keepItem ) ) )
			{
			/*
			 * No it appears not, so let's see if we have default value for that kind.
			 */
			keepItem = "";
			HXml::HNode::properties_t::iterator defaultIt = props.find( D_ATTRIBUTE_DEFAULT );
			if ( defaultIt != props.end() )
				{
				/*
				 * Alright, yes we have.
				 */
				keepItem = defaultIt->second;
				defaults.insert( kind->second, keepItem );
				}
			}
		if ( ! keepItem.is_empty() )
			{
			HXml::HNode::properties_t::iterator globalIt = props.find( D_ATTRIBUTE_GLOBAL );
			if ( ( globalIt != props.end() ) && lexical_cast<bool>( globalIt->second ) )
				{
				default_t::iterator defIt = defaults.find( kind->second );
				if ( defIt != defaults.end() )
					{
					keep_t::iterator keepIt = global.find( defIt->second );
					if ( keepIt != global.end() )
						global.erase( keepIt );
					}
				defaults.insert( kind->second, keepItem );
				global.insert( keepItem );
				}
			db.insert( keepItem );
			}
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
	static char const* const D_ATTRIBUTE_KIND = "kind";
	static HXml waste;
	static HXml::HNodeProxy root;
	static keep_t keepGlobal;
	if ( ! selfwaste )
		{
		keepGlobal.clear();
		waste.create_root( "x" );
		root = waste.get_root();
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
				build_keep_db( *del, req, keep, keepGlobal, defaults );
				selfwaste->move_node( *del );
				}
			else if ( is_kind_of( *del, D_CLASS_WASTEABLE ) )
				{
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator kindIt = props.find( D_ATTRIBUTE_KIND );
				if ( kindIt != props.end() )
					{
					if ( ( keep.find( kindIt->second ) == keep.end() )
							&& ( keepGlobal.find( kindIt->second ) == keepGlobal.end() ) )
						selfwaste->move_node( *del );
					else
						waste_children( *del, req, defaults, selfwaste );
					props.erase( kindIt );
					}
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
	static char const* const D_ATTRIBUTE_ID = "id";
	for ( HXml::HIterator it = node.begin(); it != node.end(); ++ it )
		{
		if ( (*it).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			HXml::HNode::properties_t& props = (*it).properties();
			if ( is_kind_of( props, D_CLASS_MARKABLE ) )
				{
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
	static HXml::HNodeProxy root;
	if ( ! selfwaste )
		{
		waste.create_root( "x" );
		root = waste.get_root();
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

void subst_item( HXml::HNodeProxy node, HRecordSet::iterator const& it, yaal::tools::HXml::HNodeProxy* pick )
	{
	M_PROLOG
	static char const* const D_NODE_ITEM = "item";
	static char const* const D_ATTRIBUTE_INDEX = "index";
	for ( HXml::HIterator child = node.begin(); child != node.end(); )
		{
		HXml::HIterator del = child;
		++ child;
		if ( (*del).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( (*del).get_name() == D_NODE_ITEM )
				{
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator idxIt = props.find( D_ATTRIBUTE_INDEX );
				if ( idxIt != props.end() )
					{
					HString val = it[ lexical_cast<int>( idxIt->second ) ];
					if ( child == node.end() )
						child = node.add_node( HXml::HNode::TYPE::D_CONTENT, val );
					else
						child = node.insert_node( child, HXml::HNode::TYPE::D_CONTENT, val );
					}
				pick->move_node( *del );
				}
			else
				subst_item( *del, it, pick );
			}
		}
	return;
	M_EPILOG
	}

void run_query( yaal::tools::HXml::HNodeProxy node, HDataBase::ptr_t db, yaal::tools::HXml& doc, yaal::tools::HXml::HNodeProxy* pick )
	{
	M_PROLOG
	static char const* const D_NODE_QUERY = "query";
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
				HXml::HIterator query = pick->move_node( *del );
				HXml::HNode::properties_t& props = (*query).properties();
				HXml::HNode::properties_t::iterator sqlIt = props.find( D_ATTRIBUTE_SQL );
				HXml::HIterator rowIt = (*query).begin();
				if ( ( rowIt != (*query).end() ) && ( sqlIt != props.end() ) )
					{
					HXml::HNodeProxy row = *rowIt;
					HString sql = sqlIt->second;
					if ( ! sql.is_empty() )
						{
						HRecordSet::ptr_t rs = db->query( sql );
						for ( HRecordSet::iterator it = rs->begin(); it != rs->end(); ++ it )
							{
							if ( child == node.end() )
								child = node.copy_node( row );
							else
								{
								child = node.copy_node( child, row );
								}
							subst_item( *child, it, pick );
							++ child;
							}
						}
					}
				}
			else
				run_query( *del, db, doc, pick );
			}
		}
	return;
	M_EPILOG
	}

void make_cookies( yaal::tools::HXml::HNodeProxy logic, ORequest& req )
	{
	M_PROLOG
	static char const D_NODE_COOKIE[] = "cookie";
	static char const D_ATTRIBUTE_NAME[] = "name";
	HString value;
	for ( HXml::HIterator child = logic.begin(); child != logic.end(); ++ child )
		{
		if ( (*child).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( (*child).get_name() == D_NODE_COOKIE )
				{
				HXml::HNode::properties_t& props = (*child).properties();
				HXml::HNode::properties_t::iterator nameIt = props.find( D_ATTRIBUTE_NAME );
				M_ENSURE( nameIt != props.end() );
				M_ENSURE( ! nameIt->second.is_empty() );
				if ( ! req.lookup( nameIt->second, value, ORequest::ORIGIN::D_POST | ORequest::ORIGIN::D_GET ) )
					req.update( nameIt->second, value, ORequest::ORIGIN::D_COOKIE );
				}
			}
		}
	return;
	M_EPILOG
	}

void expand_autobutton( yaal::tools::HXml::HNodeProxy node, ORequest const& req )
	{
	M_PROLOG
	static char const* const D_CLASS_AUTOBUTTON = "autobutton";
	static char const* const D_NODE_FIELDSET = "fieldset";
	static char const* const D_NODE_INPUT = "input";
	static char const* const D_ATTRIBUTE_TYPE = "type";
	static char const* const D_ATTRIBUTE_TYPE_VALUE = "hidden";
	static char const* const D_ATTRIBUTE_NAME = "name";
	static char const* const D_ATTRIBUTE_VALUE = "value";
	for ( HXml::HIterator it = node.begin(); it != node.end(); ++ it )
		{
		if ( (*it).get_type() == HXml::HNode::TYPE::D_NODE )
			{
			if ( is_kind_of( *it, D_CLASS_AUTOBUTTON ) )
				{
				HXml::HIterator fieldsetIt = (*it).begin();
				M_ENSURE( fieldsetIt != (*it).end() );
				HXml::HNodeProxy fieldset = *fieldsetIt;
				M_ENSURE( ( fieldset.get_type() == HXml::HNode::TYPE::D_NODE )
						&& ( fieldset.get_name() == D_NODE_FIELDSET ) );
				keep_t keep;
				for ( HXml::HIterator fieldIt = fieldset.begin(); fieldIt != fieldset.end(); ++ fieldIt )
					{
					if ( (*fieldIt).get_type() == HXml::HNode::TYPE::D_NODE )
						{
						HXml::HNode::properties_t& props = (*fieldIt).properties();
						HXml::HNode::properties_t::iterator nameIt = props.find( D_ATTRIBUTE_NAME );
						if ( nameIt != props.end() )
							keep.insert( nameIt->second );
						}
					}
				for ( ORequest::const_iterator reqIt = req.begin(); reqIt != req.end(); ++ reqIt )
					{
					if ( keep.find( (*reqIt).first ) == keep.end() )
						{
						keep.insert( (*reqIt).first );
						HXml::HNodeProxy input = *fieldset.add_node( HXml::HNode::TYPE::D_NODE, D_NODE_INPUT );
						HXml::HNode::properties_t& props = input.properties();
						props.insert( D_ATTRIBUTE_TYPE, D_ATTRIBUTE_TYPE_VALUE );
						props.insert( D_ATTRIBUTE_NAME, (*reqIt).first );
						props.insert( D_ATTRIBUTE_VALUE, (*reqIt).second );
						}
					}
				}
			else
				expand_autobutton( *it, req );
			}
		}
	return;
	M_EPILOG
	}

}

}

