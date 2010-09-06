#ifndef CROSS_SC_POSIXRS232PORT_H_INCLUDED
#define CROSS_SC_POSIXRS232PORT_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// posix.rs232port.h: interface for the rs232port class (POSIX API).
// (c) Aug 26, 2010 Oleg N. Peregudov
//	Sep 4, 2010	- postTerminate is now synchronized with the receive thread
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/basic.rs232port.h>
#include <crs/condition_variable.h>
#include <crs/socket.h>
#include <termios.h>

#define NOPARITY		0
#define ODDPARITY		1
#define EVENPARITY	2
#define MARKPARITY	3
#define SPACEPARITY	4

namespace sc {

//
// class posixRS232port
//
class CROSS_EXPORT posixRS232port : public basicRS232port
{
protected:
	int		m_hCommPort;
	termios	m_portOptions;
	int		ipcPipeEnd [ 2 ];
	char		pipeInBuf [ 16 ];
	char *	pipeInBufPtr;
	
	CrossClass::LockType			mutexTerminate;
	CrossClass::cConditionVariable	condTerminate;
	bool						flagTerminate;
	
	virtual void UpdateConnection ();
	
	bool	wait4write ();
	bool	checkTerminate ();
	
public:
	void open ( const std::string & portName = "/dev/ttyS0", const size_t baudRate = B115200 );
	
	virtual void close ();
	virtual void write ( const char * lpBuf, const size_t dwToWrite );
	virtual void postTerminate ();
	
	virtual bool receive ();
	// returns false when termination is requested
	
	posixRS232port ( const size_t inBufSize = 2048 );
	virtual ~posixRS232port();
};

} // namespace sc
#endif // CROSS_SC_POSIXRS232PORT_H_INCLUDED

