/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	applicationserver.hxx - this file is integral part of `hector' project.

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

#ifndef __APPLICATIONSERVER_H
#define __APPLICATIONSERVER_H

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hmap.hxx>
#include <yaal/tools/hxml.hxx>
#include <yaal/tools/signals.hxx>
#include "server.hxx"
#include "application.hxx"

namespace hector
{

struct OProcessor
	{
	yaal::hcore::HString f_oBinaryPath;
	yaal::tools::HPlugin::ptr_t f_oActiveX;
	HApplication::ptr_t f_oApplication;
	OProcessor( yaal::hcore::HString const& path ) : f_oBinaryPath( path ), f_oActiveX(), f_oApplication() {}
	static OProcessor get_instance( yaal::hcore::HString const&, yaal::hcore::HString const& );
	void reload_binary( void );
	};

class HApplicationServer : public HServer
	{
	typedef yaal::hcore::HMap<yaal::hcore::HString, OProcessor> applications_t;
	typedef yaal::hcore::HMap<int, yaal::hcore::HSocket::ptr_t> pending_t;
	applications_t f_oApplications;
	pending_t f_oPending;
	yaal::tools::HXml f_oConfiguration;
	yaal::hcore::HString f_oDefaultApplication;
	yaal::hcore::HPipe f_oSigChildEvent;
public:
	HApplicationServer( void );
	virtual ~HApplicationServer( void );
	void start( void );
	void stop( void );
	void run( void );
	int on_sigchild( int );
	int process_sigchild( int );
protected:
	virtual void do_service_request( ORequest& );
	virtual void do_reload( yaal::hcore::HSocket::ptr_t&, yaal::hcore::HString const& );
	virtual void do_status( yaal::hcore::HSocket::ptr_t& );
	void clean_request( int );
private:
	void read_configuration( yaal::tools::HXml::HConstNodeProxy const& );
	void read_applications( yaal::tools::HXml::HConstNodeProxy const& );
	};

}

#endif /* not __APPLICATIONSERVER_H */

