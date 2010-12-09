#ifndef CROSS_BASICNETPOINT_H_INCLUDED
#define CROSS_BASICNETPOINT_H_INCLUDED 1
// (c) Jan 28, 2009 Oleg N. Peregudov
// 04/23/2009 - Win/Posix defines
// 08/24/2010 - new server termination algorithm
// 11/30/2010 - new name for termination method
//              new implementation
// 12/04/2010 - stored socket address

#include <crs/socket.h>

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
	cSocket 		_socket;
	cSockAddr		_sockAddress;
	basicNetPoint* *	_clientList;
	size_t		_nClients,
				_nClientsAllocated;
	
	basicNetPoint ( cSocket & );
	
	void addClient ( basicNetPoint * );
	void removeClient ( const size_t );
	void clientReceive ( const size_t );
	void clientTransmit ( const size_t );
	void disconnectAll ();
	
protected:
	virtual void setNonBlock ();
	
	virtual void transmit ();
	virtual void receive ();
	
	virtual basicNetPoint * handleNewConnection ( cSocket &, const cSockAddr & );
	virtual size_t enumerateDescriptors ();
	virtual void buildSelectList ();
	virtual bool handleDisconnect ( basicNetPoint * );
	virtual bool checkTerminate ();
	
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
	NETPOINT_EXPORTED_EXCEPTION( shutdown_error );
	NETPOINT_EXPORTED_EXCEPTION( pipe_open_error );
	NETPOINT_EXPORTED_EXCEPTION( pipe_close_error );
	
	basicNetPoint ();
	virtual ~basicNetPoint ();
	
	void startServer ( const unsigned short int portNo );
	virtual void bindSocket ( const cSockAddr & );
	virtual void clientConnect ( const cSockAddr & );
	
	virtual void postTerminate ();
	virtual bool clientSendRecv ();
	virtual bool serverSendRecv ();
	
	cSocket & getSocket ();
};

} // namespace CrossClass
#undef NETPOINT_EXPORTED_EXCEPTION
#endif // CROSS_BASICNETPOINT_H_INCLUDED

