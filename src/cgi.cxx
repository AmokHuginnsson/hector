/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  cgi.cxx - this file is integral part of `hector' project.

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
#include <iostream>

#include <yaal/hcore/htokenizer.hxx>
#include <yaal/hcore/hfile.hxx>
#include <yaal/tools/stringalgo.hxx>
#include <yaal/tools/hxml.hxx>
M_VCSID( "$Id: " __ID__ " $" )
#include "cgi.hxx"
#include "application.hxx"
#include "setup.hxx"

using namespace yaal;
using namespace yaal::hcore;
using namespace yaal::tools;
using namespace yaal::dbwrapper;

namespace hector {

namespace cgi {

static HString const ATTRIBUTE_ID( "id" );
static HString const ATTRIBUTE_CLASS( "class" );
static HString const ATTRIBUTE_USER( "user" );
static HString const ATTRIBUTE_GROUP( "group" );
static HString const ATTRIBUTE_MODE( "mode" );
static HString const CLASS_WASTEABLE( "wasteable" );
static HString const CLASS_AUTOBUTTON( "autobutton" );

bool is_in_attribute( yaal::tools::HXml::HNode::properties_t const& props, HString const& attribute, HString const& element ) {
	M_PROLOG
	static char const* const ELEMENT_SEPARATOR = " \t";
	HXml::HNode::properties_t::const_iterator attrIt = props.find( attribute );
	bool is = false;
	if ( attrIt != props.end() ) {
		int idx = 0;
		bool leftLimit = ( ( idx = static_cast<int>( attrIt->second.find( element ) ) ) >= 0 )
			&& ( ! idx || ::strchr( ELEMENT_SEPARATOR, attrIt->second[ idx - 1 ] ) );
		if ( leftLimit ) {
			int len = static_cast<int>( element.get_length() );
			is = ( ( idx + len ) >= attrIt->second.get_length() ) || ::strchr( ELEMENT_SEPARATOR, attrIt->second[ idx + len ] );
		}
	}
	return ( is );
	M_EPILOG
}

bool is_kind_of( yaal::tools::HXml::HNode::properties_t const& props, HString const& kind ) {
	M_PROLOG
	return ( is_in_attribute( props, ATTRIBUTE_CLASS, kind ) );
	M_EPILOG
}

bool has_attribute( yaal::tools::HXml::HNode::properties_t const& props, HString const& attribute ) {
	M_PROLOG
	HXml::HNode::properties_t::const_iterator attrIt = props.find( attribute );
	return ( attrIt != props.end() );
	M_EPILOG
}

bool is_in_attribute( yaal::tools::HXml::HConstNodeProxy const& node, HString const& attribute, HString const& element ) {
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::NODE );
	HXml::HNode::properties_t const& props = node.properties();
	return ( is_in_attribute( props, attribute, element ) );
	M_EPILOG
}

bool is_kind_of( yaal::tools::HXml::HConstNodeProxy const& node, HString const& kind ) {
	M_PROLOG
	return ( is_in_attribute( node, ATTRIBUTE_CLASS, kind ) );
	M_EPILOG
}

bool has_attribute( yaal::tools::HXml::HConstNodeProxy const& node, HString const& attribute ) {
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::NODE );
	HXml::HNode::properties_t const& props = node.properties();
	return ( has_attribute( props, attribute ) );
	M_EPILOG
}

HString const& get_owner_user( HXml::HConstNodeProxy const& node );
HString const& get_owner_user( HXml::HConstNodeProxy const& node ) {
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::NODE );
	return ( get_optional_value_or( xml::try_attr_val( node, ATTRIBUTE_USER ), HString() ) );
	M_EPILOG
}

HString const& get_owner_group( HXml::HConstNodeProxy const& node );
HString const& get_owner_group( HXml::HConstNodeProxy const& node ) {
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::NODE );
	return ( get_optional_value_or( xml::try_attr_val( node, ATTRIBUTE_GROUP ), HString() ) );
	M_EPILOG
}

int get_permissions( HXml::HConstNodeProxy const& node );
int get_permissions( HXml::HConstNodeProxy const& node ) {
	M_PROLOG
	M_ASSERT( node.get_type() == HXml::HNode::TYPE::NODE );
	xml::value_t val( xml::try_attr_val( node, ATTRIBUTE_MODE ) );
	return ( val ? lexical_cast<int>( *val ) : -1 );
	M_EPILOG
}

namespace {

void update_security_context( OSecurityContext& securityContext_, HXml::HConstNodeProxy const& node_ ) {
	M_PROLOG
	M_ASSERT( node_.get_type() == HXml::HNode::TYPE::NODE );
	xml::value_t user( xml::try_attr_val( node_, ATTRIBUTE_USER ) );
	xml::value_t group( xml::try_attr_val( node_, ATTRIBUTE_GROUP ) );
	xml::value_t mode( xml::try_attr_val( node_, ATTRIBUTE_MODE ) );
	if ( user ) {
		securityContext_._user = *user;
	}
	if ( group ) {
		securityContext_._group = *group;
	}
	if ( mode ) {
		securityContext_._mode = static_cast<ACCESS::mode_t>( lexical_cast<int>( *mode ) );
	}
	return;
	M_EPILOG
}

void build_keep_db( HXml::HNodeProxy keep, ORequest const& req, keep_t& db, keep_t& global, default_t& defaults ) {
	M_PROLOG
	static HString const NODE_KEEP_RULE( "rule" );
	static HString const ATTRIBUTE_KIND( "kind" );
	static HString const ATTRIBUTE_DEFAULT( "default" );
	static HString const ATTRIBUTE_VALID( "valid" );
	static HString const ATTRIBUTE_GLOBAL( "global" );
	/*
	 * Look thru all <rule />'s.
	 */
	for ( HXml::HIterator it( keep.begin() ), end( keep.end() ); it != end; ++ it ) {
		M_ENSURE( (*it).get_type() == HXml::HNode::TYPE::NODE );
		M_ENSURE( (*it).get_name() == NODE_KEEP_RULE );
		HXml::HNode::properties_t& props = (*it).properties();
		HXml::HNode::properties_t::iterator kind = props.find( ATTRIBUTE_KIND );
		/*
		 * All <rule />'s must have kind="" attribute.
		 */
		M_ENSURE( kind != props.end() );
		HString keepItem( "" );
		/*
		 * Let's find out if request have that `kind' specified.
		 */
		if ( req.lookup( kind->second, keepItem )
				|| ( has_attribute( props, ATTRIBUTE_VALID )
					&& ! is_in_attribute( props, ATTRIBUTE_VALID, keepItem ) ) ) {
			/*
			 * No it appears not, so let's see if we have default value for that kind.
			 */
			keepItem = "";
			HXml::HNode::properties_t::iterator defaultIt = props.find( ATTRIBUTE_DEFAULT );
			if ( defaultIt != props.end() ) {
				/*
				 * Alright, yes we have.
				 */
				keepItem = defaultIt->second;
				defaults[ kind->second ] = keepItem;
			}
		}
		if ( ! keepItem.is_empty() ) {
			HXml::HNode::properties_t::iterator globalIt = props.find( ATTRIBUTE_GLOBAL );
			if ( ( globalIt != props.end() ) && lexical_cast<bool>( globalIt->second ) ) {
				default_t::iterator defIt = defaults.find( kind->second );
				if ( defIt != defaults.end() ) {
					keep_t::iterator keepIt = global.find( defIt->second );
					if ( keepIt != global.end() )
						global.erase( keepIt );
				}
				defaults[ kind->second ] = keepItem;
				global.insert( keepItem );
			}
			db.insert( keepItem );
		}
	}
	return;
	M_EPILOG
}

}

void waste_children( yaal::tools::HXml::HNodeProxy node,
		ORequest const& req, default_t& defaults,
		HXml::HNodeProxy* selfwaste ) {
	M_PROLOG
	static HString const NODE_KEEP( "keep" );
	static HString const ATTRIBUTE_KIND( "kind" );
	static HXml waste;
	static HXml::HNodeProxy root;
	static keep_t keepGlobal;
	if ( ! selfwaste ) {
		keepGlobal.clear();
		waste.create_root( "x" );
		root = waste.get_root();
		selfwaste = &root;
	}
	keep_t keep;
	for ( HXml::HIterator it = node.begin(); it != node.end(); ) {
		HXml::HIterator del( it );
		++ it;
		if ( (*del).get_type() == HXml::HNode::TYPE::NODE ) {
			if ( (*del).get_name() == NODE_KEEP ) {
				build_keep_db( *del, req, keep, keepGlobal, defaults );
				selfwaste->move_node( *del );
			} else if ( is_kind_of( *del, CLASS_WASTEABLE ) ) {
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator kindIt = props.find( ATTRIBUTE_KIND );
				if ( kindIt != props.end() ) {
					HTokenizer tok( kindIt->second, "," );
					bool wasted( false );
					for ( HTokenizer::HIterator t( tok.begin() ), te( tok.end() ); t != te; ++ t ) {
						if ( ( keep.find( *t ) == keep.end() )
								&& ( keepGlobal.find( *t ) == keepGlobal.end() ) ) {
							selfwaste->move_node( *del );
							wasted = true;
							break;
						}
					}
					if ( ! wasted )
						waste_children( *del, req, defaults, selfwaste );
					props.erase( kindIt );
				} else {
					M_ENSURE( ! "wasteable node must have 'kind' attribute" );
				}
			} else
				waste_children( *del, req, defaults, selfwaste );
		}
	}
	return;
	M_EPILOG
}

void mark_children( yaal::tools::HXml::HNodeProxy node,
		ORequest const& req, default_t const& defaults, HXml& doc ) {
	M_PROLOG
	static HString const CLASS_MARKABLE( "markable" );
	static HString const CLASS_CURRENT( " current" );
	for ( HXml::HIterator it( node.begin() ), end( node.end() ); it != end; ++ it ) {
		if ( (*it).get_type() == HXml::HNode::TYPE::NODE ) {
			HXml::HNode::properties_t& props = (*it).properties();
			if ( is_kind_of( props, CLASS_MARKABLE ) ) {
				HXml::HNode::properties_t::iterator id = props.find( ATTRIBUTE_ID );
				if ( id != props.end() ) {
					HString subject = id->second.mid( id->second.find( "-" ) + 1 ); /* + 1 for '-' character */
					if ( ! subject.is_empty() ) {
						HString object;
						if ( req.lookup( subject, object ) ) {
							default_t::const_iterator defaultIt = defaults.find( subject );
							if ( defaultIt != defaults.end() )
								object = defaultIt->second;
						}
						if ( ! object.is_empty() ) {
							HXml::HNodeProxy mark = doc.get_element_by_id( subject + "-" + object );
							if ( !! mark )
								mark.properties()[ ATTRIBUTE_CLASS ] += CLASS_CURRENT;
						}
					}
				}
			} else
				mark_children( *it, req, defaults, doc );
		}
	}
	return;
	M_EPILOG
}

void move_children( yaal::tools::HXml::HNodeProxy node, ORequest const& req,
		yaal::tools::HXml& doc, HXml::HNodeProxy* selfwaste ) {
	M_PROLOG
	static HString const NODE_MOVE( "move" );
	static HString const ATTRIBUTE_TO( "to" );
	static HXml waste;
	static HXml::HNodeProxy root;
	if ( ! selfwaste ) {
		waste.create_root( "x" );
		root = waste.get_root();
		selfwaste = &root;
	}
	for ( HXml::HIterator it = node.begin(); it != node.end(); ) {
		HXml::HIterator del = it;
		++ it;
		if ( (*del).get_type() == HXml::HNode::TYPE::NODE ) {
			if ( (*del).get_name() == NODE_MOVE ) {
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator toIt = props.find( ATTRIBUTE_TO );
				if ( toIt != props.end() ) {
					HXml::HNodeSet to( doc.get_elements_by_path( toIt->second ) );
					if ( ! to.is_empty() ) {
						for ( HXml::HIterator moveIt = (*del).begin(); moveIt != (*del).end(); ) {
							HXml::HIterator move = moveIt;
							++ moveIt;
							to[0].move_node( *move );
						}
					}
				}
				selfwaste->move_node( *del );
			} else
				move_children( *del, req, doc );
		}
	}
	return;
	M_EPILOG
}

namespace {

void subst_item( HXml::HNodeProxy node, HRecordSet::iterator const& it, yaal::tools::HXml::HNodeProxy* pick ) {
	M_PROLOG
	static HString const NODE_ITEM( "item" );
	static HString const ATTRIBUTE_INDEX( "index" );
	for ( HXml::HIterator child = node.begin(); child != node.end(); ) {
		HXml::HIterator del = child;
		++ child;
		if ( (*del).get_type() == HXml::HNode::TYPE::NODE ) {
			if ( (*del).get_name() == NODE_ITEM ) {
				HXml::HNode::properties_t& props = (*del).properties();
				HXml::HNode::properties_t::iterator idxIt = props.find( ATTRIBUTE_INDEX );
				if ( idxIt != props.end() ) {
					HString val( *it[ lexical_cast<int>( idxIt->second ) ] );
					if ( child == node.end() )
						child = node.add_node( HXml::HNode::TYPE::CONTENT, val );
					else
						child = node.insert_node( child, HXml::HNode::TYPE::CONTENT, val );
				}
				pick->move_node( *del );
			} else
				subst_item( *del, it, pick );
		}
	}
	return;
	M_EPILOG
}

}

void run_query( yaal::tools::HXml::HNodeProxy node, HDataBase::ptr_t db, yaal::tools::HXml& doc, yaal::tools::HXml::HNodeProxy* pick ) {
	M_PROLOG
	static HString const NODE_QUERY( "query" );
	static HString const ATTRIBUTE_SQL( "sql" );
	static HXml waste;
	if ( ! pick ) {
		waste.create_root( "x" );
		HXml::HNodeProxy root = waste.get_root();
		pick = &root;
	}
	for ( HXml::HIterator child = node.begin(); child != node.end(); ) {
		HXml::HIterator del = child;
		++ child;
		if ( (*del).get_type() == HXml::HNode::TYPE::NODE ) {
			if ( (*del).get_name() == NODE_QUERY ) {
				HXml::HIterator query = pick->move_node( *del );
				HXml::HNode::properties_t& props = (*query).properties();
				HXml::HNode::properties_t::iterator sqlIt = props.find( ATTRIBUTE_SQL );
				HXml::HIterator rowIt = (*query).begin();
				if ( ( rowIt != (*query).end() ) && ( sqlIt != props.end() ) ) {
					HXml::HNodeProxy row = *rowIt;
					HString sql = sqlIt->second;
					if ( ! sql.is_empty() ) {
						HRecordSet::ptr_t rs = db->execute_query( sql );
						for ( HRecordSet::iterator it = rs->begin(); it != rs->end(); ++ it ) {
							if ( child == node.end() )
								child = node.copy_node( row );
							else {
								child = node.copy_node( child, row );
							}
							subst_item( *child, it, pick );
							++ child;
						}
					}
				}
			} else
				run_query( *del, db, doc, pick );
		}
	}
	return;
	M_EPILOG
}

void expand_autobutton( yaal::tools::HXml::HNodeProxy node, ORequest const& req ) {
	M_PROLOG
	static HString const NODE_FIELDSET( "fieldset" );
	static HString const NODE_INPUT( "input" );
	static HString const NODE_A( "a" );
	static HString const ATTRIBUTE_HREF( "href" );
	static HString const ATTRIBUTE_TYPE( "type" );
	static HString const ATTRIBUTE_TYPE_VALUE( "hidden" );
	static HString const ATTRIBUTE_NAME( "name" );
	static HString const ATTRIBUTE_VALUE( "value" );
	static char const QUERY_SEPARATOR( '?' );
	static char const PARAM_SEPARATOR( '&' );
	static char const VALUE_SEPARATOR( '=' );
	for ( HXml::HIterator it( node.begin() ); it != node.end(); ++ it ) {
		HXml::HNodeProxy n( *it );
		if ( n.get_type() == HXml::HNode::TYPE::NODE ) {
			if ( is_kind_of( *it, CLASS_AUTOBUTTON ) ) {
				keep_t keep;
				if ( n.get_name() == NODE_A ) {
					HString& href( n.properties().at( ATTRIBUTE_HREF ) );
					int long querySepPos( href.find( QUERY_SEPARATOR ) );
					bool haveParams( false );
					if ( querySepPos != HString::npos ) {
						typedef HArray<HString> params_t;
						params_t params( string::split<params_t>( href.substr( querySepPos + 1 ), PARAM_SEPARATOR ) );
						haveParams = ! params.is_empty();
						for ( HString const& s : params ) {
							keep.insert( s.substr( 0, s.find( VALUE_SEPARATOR ) ) );
						}
					}
					for ( ORequest::const_iterator reqIt( req.begin() ), end( req.end() ); reqIt != end; ++ reqIt ) {
						if ( keep.insert( (*reqIt).first ).second ) {
							if ( querySepPos == HString::npos ) {
								href.append( QUERY_SEPARATOR );
								querySepPos = 0;
							} else if ( haveParams && ( href.back() != PARAM_SEPARATOR ) ) {
								href.append( PARAM_SEPARATOR );
							}
							haveParams = true;
							href.append( (*reqIt).first ).append( VALUE_SEPARATOR ).append( (*reqIt).second );
						}
					}
				} else {
					HXml::HIterator fieldsetIt = n.begin();
					M_ENSURE( fieldsetIt != n.end() );
					HXml::HNodeProxy fieldset = *fieldsetIt;
					M_ENSURE( ( fieldset.get_type() == HXml::HNode::TYPE::NODE )
							&& ( fieldset.get_name() == NODE_FIELDSET ) );
					for ( HXml::HIterator fieldIt( fieldset.begin() ), end( fieldset.end() ); fieldIt != end; ++ fieldIt ) {
						if ( (*fieldIt).get_type() == HXml::HNode::TYPE::NODE ) {
							HXml::HNode::properties_t& props = (*fieldIt).properties();
							HXml::HNode::properties_t::iterator nameIt = props.find( ATTRIBUTE_NAME );
							if ( nameIt != props.end() ) {
								keep.insert( nameIt->second );
							}
						}
					}
					for ( ORequest::const_iterator reqIt( req.begin() ), end( req.end() ); reqIt != end; ++ reqIt ) {
						if ( keep.insert( (*reqIt).first ).second ) {
							HXml::HNodeProxy input = *fieldset.add_node( HXml::HNode::TYPE::NODE, NODE_INPUT );
							HXml::HNode::properties_t& props = input.properties();
							props[ ATTRIBUTE_TYPE ] = ATTRIBUTE_TYPE_VALUE;
							props[ ATTRIBUTE_NAME ] = (*reqIt).first;
							props[ ATTRIBUTE_VALUE ] = (*reqIt).second;
						}
					}
				}
			} else {
				expand_autobutton( n, req );
			}
		}
	}
	return;
	M_EPILOG
}

/* We keep node only if 'read' bit is set for user.
 */
void apply_acl( yaal::tools::HXml::HNodeProxy node_,
		ORequest const& req_, OSecurityContext const& securityContext_,
		HSession const& session_,
		HXml::HNodeProxy* selfwaste_ ) {
	M_PROLOG
	static HXml waste;
	static HXml::HNodeProxy root;
	OSecurityContext securityContext( securityContext_ );
	if ( ! selfwaste_ ) {
		waste.create_root( "x" );
		root = waste.get_root();
		selfwaste_ = &root;
	}
	update_security_context( securityContext, node_ );
	if ( ! has_access( ACCESS::READ, session_, securityContext ) )
		selfwaste_->move_node( node_ );
	else {
		for ( HXml::HIterator it( node_.begin() ); it != node_.end(); ) {
			if ( (*it).get_type() == HXml::HNode::TYPE::NODE ) {
				HXml::HIterator probe( it );
				++ it;
				apply_acl( *probe, req_, securityContext, session_, selfwaste_ );
			} else
				++ it;
		}
	}
	return;
	M_EPILOG
}

void consistency_check( yaal::tools::HXml::HNodeProxy node_ ) {
	M_PROLOG
	ORequest::value_t optMode( xml::try_attr_val( node_, ATTRIBUTE_MODE ) );
	if ( optMode ) {
		int mode( lexical_cast<int>( *optMode ) );
		M_ENSURE_EX( ( is_octal( *optMode ) && ( mode >= 0 ) && ( mode <= 0777 ) ), *optMode );
	}
	for ( HXml::HIterator child( node_.begin() ), endChild( node_.end() ); child != endChild; ++ child ) {
		if ( (*child).get_type() == HXml::HNode::TYPE::NODE )
			consistency_check( *child );
	}
	return;
	M_EPILOG
}

void prepare_logic(  HApplication* app_, yaal::tools::HXml::HNodeProxy node_ ) {
	M_PROLOG
	static HString const NODE_FORM( "h-form" );
	static HString const NODE_INPUT( "h-input" );
	static HString const NODE_VERIFY( "verify" );
	HString name;
	for ( HXml::HIterator child( node_.begin() ), endChild( node_.end() ); child != endChild; ++ child ) {
		if ( (*child).get_type() == HXml::HNode::TYPE::NODE ) {
			name = (*child).get_name();
			if ( name == NODE_FORM ) {
				ORequest::value_t optId( xml::try_attr_val( *child, ATTRIBUTE_ID ) );
				M_ENSURE_EX( !!optId, "h-from must have an `id' attribute: "_ys.append( (*child).get_line() ) );
				HForm form;
				for ( HXml::HIterator it( (*child).begin() ); it != (*child).end(); ) {
					HXml::HIterator del( it );
					++ it;
					if ( (*del).get_type() == HXml::HNode::TYPE::NODE ) {
						name = (*del).get_name();
						if ( name == NODE_INPUT ) {
							out << "input" << endl;
						} else if ( name == NODE_VERIFY ) {
							M_ENSURE_EX( (*del).has_childs(), "verificator needs to have a body: "_ys.append( (*del).get_line() ) );
							app_->add_verificator( *optId, (*(*del).begin()).get_value() );
							(*child).remove_node( del );
							out << "verify" << endl;
						}
					}
				}
				out << NODE_FORM << ": " << get_optional_value_or<HString>( optId, "(nuil)" ) << endl;
				app_->add_form( make_pair( get_optional_value_or<HString>( optId, "(nuil)" ), form ) );
			} else
				prepare_logic( app_, *child );
		}
	}
	return;
	M_EPILOG
}

void make_cookies( yaal::tools::HXml::HNodeProxy logic, ORequest& req ) {
	M_PROLOG
	static HString const NODE_COOKIE( "cookie" );
	static HString const ATTRIBUTE_NAME( "name" );
	HString value;
	for ( HXml::HIterator child( logic.begin() ), endChild( logic.end() ); child != endChild; ++ child ) {
		if ( (*child).get_type() == HXml::HNode::TYPE::NODE ) {
			if ( (*child).get_name() == NODE_COOKIE ) {
				HXml::HNode::properties_t& props( (*child).properties() );
				HXml::HNode::properties_t::iterator nameIt( props.find( ATTRIBUTE_NAME ) );
				M_ENSURE( nameIt != props.end() );
				M_ENSURE( ! nameIt->second.is_empty() );
				if ( ! req.lookup( nameIt->second, value, ORequest::ORIGIN::POST | ORequest::ORIGIN::GET ) )
					req.update( nameIt->second, value, ORequest::ORIGIN::COOKIE );
			}
		}
	}
	return;
	M_EPILOG
}

bool has_access( ACCESS::type_t accessType_, HSession const& session_, OSecurityContext const& securityContext_ ) {
	M_PROLOG
	bool access( false );
	if ( session_.get_user() == securityContext_._user ) {
		access = ( ( securityContext_._mode & ( static_cast<int unsigned>( accessType_ ) << ACCESS::USER ) ) != ACCESS::NONE );
	} else if ( session_.get_groups().count( securityContext_._group ) > 0 ) {
		access = ( ( securityContext_._mode & ( static_cast<int unsigned>( accessType_ ) << ACCESS::GROUP ) ) != ACCESS::NONE );
	} else {
		access = ( ( securityContext_._mode & ( static_cast<int unsigned>( accessType_ ) << ACCESS::OTHER ) ) != ACCESS::NONE );
	}
	return ( access );
	M_EPILOG
}

void clean( yaal::tools::HXml::HNodeProxy node_ ) {
	M_PROLOG
	HXml::HNode::properties_t& props( node_.properties() );
	props.erase( ATTRIBUTE_MODE );
	props.erase( ATTRIBUTE_USER );
	props.erase( ATTRIBUTE_GROUP );
	HXml::HNode::properties_t::iterator classProp( props.find( ATTRIBUTE_CLASS ) );
	if ( classProp != props.end() ) {
		HString& classVal( classProp->second );
		int long valIdx( classVal.find( CLASS_WASTEABLE ) );
		if ( valIdx != HString::npos )
			classVal.erase( valIdx - ( valIdx > 0 ? 1 : 0 ), CLASS_WASTEABLE.get_length() + 1 );
		valIdx = classVal.find( CLASS_AUTOBUTTON );
		if ( valIdx != HString::npos )
			classVal.erase( valIdx - ( valIdx > 0 ? 1 : 0 ), CLASS_AUTOBUTTON.get_length() + 1 );
		if ( classVal.is_empty() )
			props.erase( classProp );
	}
	for ( HXml::HIterator it( node_.begin() ); it != node_.end(); ++ it ) {
		if ( (*it).get_type() == HXml::HNode::TYPE::NODE ) {
			clean( *it );
		}
	}
	return;
	M_EPILOG
}

}

}

