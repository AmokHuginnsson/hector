/*
---           `hector' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

	applicationserver.h - this file is integral part of `hector' project.

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

#include <yaal/hcore/hstring.h>
#include <yaal/hcore/hmap.h>
#include <yaal/tools/hxml.h>
#include "server.h"
#include "application.h"

namespace hector
{

class HApplicationServer : public HServer
	{
	typedef yaal::hcore::HMap<yaal::hcore::HString, HApplication::ptr_t> applications_t;
	applications_t f_oApplications;
	yaal::tools::HXml f_oConfiguration;
public:
	HApplicationServer( void );
	virtual ~HApplicationServer( void );
	void start( void );
	void stop( void );
	void run( void );
protected:
	virtual void do_service_request( ORequest& );
	};

}

#endif /* not __APPLICATIONSERVER_H */

