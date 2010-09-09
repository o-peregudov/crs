#ifndef CROSS_BASICNETPOINT_H_INCLUDED
#define CROSS_BASICNETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 24, 2010 - new server termination algorithm

#include <crs/socket.h>
#include <crs/handle.h>
#include <cerrno>
#include <memory>
#include <vector>

#define NETPOINT_EXPORTED_EXCEPTION( exception_name )			\
	struct CROSS_EXPORT exception_name : std::runtime_error	\
	{										\
		exception_name ( const std::string & what_arg )		\
			: std::runtime_error( what_arg ) { }		\
	}

namespace CrossClass {

class CROSS_EXPORT basicNetPoint
{
protected:
	struct client_handler;
	friend struct client_handler;
	
	fd_set readset,
		 writeset,
		 exceptset;
	cSocket::host_socket_type highsock;
	std::vector<cHandle<basicNetPoint> > clientList;
	cTimeVal selectTimeOut;
	
	struct client_handler
	{
		basicNetPoint * base;
		
		client_handler ( basicNetPoint * peer )
			: base( peer )
		{ }
		
		void operator () ( cHandle<basicNetPoint> & );
	};
	
protected:
	cSocket _socket;
	
	basicNetPoint ( cSocket & );
	
	virtual void setNonBlock ();
	
	virtual void transmit ();
	virtual void receive ();
	virtual void except ();
	
	virtual cHandle<basicNetPoint> handleNewConnection ( cSocket &, const cSockAddr & );
	virtual bool doServerJob ( const bool idleState );
	virtual bool doHandleClient ( basicNetPoint & );
	virtual bool handleDisconnect ( basicNetPoint & );
	
	virtual void logGracefulDisconnect ();
	virtual void logRuntimeError ( const std::string & msg );
	virtual void logUnhandledError ();
	
	virtual void bindSocket ( const cSockAddr & );
	virtual timeval * onStartServer ();
	virtual void buildSelectList ();
	virtual bool preCheckTerminate ();
	virtual bool postCheckTerminate ();
	
public:
	NETPOINT_EXPORTED_EXCEPTION( socket_allocation_error );
	NETPOINT_EXPORTED_EXCEPTION( socket_select_error );
	NETPOINT_EXPORTED_EXCEPTION( socket_options_error );
	NETPOINT_EXPORTED_EXCEPTION( socket_listen_error );
	NETPOINT_EXPORTED_EXCEPTION( socket_bind_error );
	NETPOINT_EXPORTED_EXCEPTION( socket_connect_error );
	NETPOINT_EXPORTED_EXCEPTION( server_startup_error );
	NETPOINT_EXPORTED_EXCEPTION( read_crc_error );
	NETPOINT_EXPORTED_EXCEPTION( read_error );
	NETPOINT_EXPORTED_EXCEPTION( write_error );
	NETPOINT_EXPORTED_EXCEPTION( cancel_read );
	NETPOINT_EXPORTED_EXCEPTION( cancel_write );
	NETPOINT_EXPORTED_EXCEPTION( end_of_file );
	
	basicNetPoint ();
	virtual ~basicNetPoint ();
	
	void startServer ( const unsigned short int portNo );
	void startServer ( const cSockAddr & );
	
	virtual void terminateServer ();
	virtual void clientConnect ( const cSockAddr & );
	void clientSendRecv ();
	
	cSocket & getSocket ();
};

} // namespace CrossClass
#undef NETPOINT_EXPORTED_EXCEPTION
#endif // CROSS_BASICNETPOINT_H_INCLUDED

