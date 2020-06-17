/* Read hector/LICENSE.md file for copyright and licensing information. */

#ifndef APPLICATIONSERVER_HXX_INCLUDED
#define APPLICATIONSERVER_HXX_INCLUDED 1

#include <yaal/hcore/hstring.hxx>
#include <yaal/hcore/hmap.hxx>
#include <yaal/tools/hxml.hxx>
#include <yaal/tools/signals.hxx>
#include <yaal/dbwrapper/hdatabase.hxx>

#include "server.hxx"
#include "hactivex.hxx"
#include "session.hxx"

namespace hector {

class HApplicationServer : public HServer {
	typedef yaal::hcore::HMap<yaal::hcore::HString, HActiveX> applications_t;
	typedef yaal::hcore::HHashMap<yaal::hcore::HString, yaal::dbwrapper::HDataBase::ptr_t> db_connections_t;
	typedef yaal::hcore::HMap<int, yaal::hcore::HStreamInterface::ptr_t> pending_t;
	typedef yaal::tools::HOptional<HSession&> session_t;
	applications_t _applications;
	pending_t _pending;
	yaal::tools::HXml _configuration;
	yaal::hcore::HString _defaultApplication;
	yaal::hcore::HPipe _sigChildEvent;
	db_connections_t _dbConnections;
public:
	HApplicationServer( void );
	virtual ~HApplicationServer( void );
	void start( void );
	void stop( void );
	int on_sigchild( int );
	void process_sigchild( yaal::tools::HIODispatcher::stream_t&, yaal::hcore::system::IO_EVENT_TYPE );
	yaal::dbwrapper::HDataBase::ptr_t get_db_connection( yaal::hcore::HString const& );
protected:
	virtual void do_service_request( ORequest& ) override;
	virtual void do_restart( yaal::hcore::HStreamInterface::ptr_t, yaal::hcore::HString const& ) override;
	virtual void do_reload( yaal::hcore::HStreamInterface::ptr_t, yaal::hcore::HString const& ) override;
	virtual void do_status( yaal::hcore::HStreamInterface::ptr_t& ) override;
	session_t handle_session( ORequest&, HApplication::sessions_t& );
	void clean_request( int );
private:
	void read_configuration( yaal::tools::HXml::HConstNodeProxy const& );
	void read_applications( yaal::tools::HXml::HConstNodeProxy const& );
};

typedef yaal::hcore::HExceptionT<HApplicationServer> HApplicationServerException;

}

#endif /* #ifndef APPLICATIONSERVER_HXX_INCLUDED */

