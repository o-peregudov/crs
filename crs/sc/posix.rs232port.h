#ifndef CROSS_SC_POSIXRS232PORT_H_INCLUDED
#define CROSS_SC_POSIXRS232PORT_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// posix.rs232port.h: interface for the rs232port class (POSIX API).
// (c) Aug 26, 2010 Oleg N. Peregudov
//	09/04/2010	postTerminate is now synchronized with the receive thread
//	09/09/2010	baudrate constant selector
//	09/20/2010	non-blocking postTerminate
//	12/03/2011	advanced port open
//	2012/05/10	port polling is now moved to a separate class
//	2012/07/10	callback function for eventually port disconnection
//			or other port failure
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/basic.rs232port.h>
#include <crs/eveloop.h>
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
class CROSS_EXPORT posixRS232port
	: public basicRS232port
	, public CrossClass::event_descriptor
{
protected:
	int		m_hCommPort;
	termios	m_portOptions;
	
	virtual void UpdateConnection ();
	
	const size_t 		bufSize;
	char *			outBuf;
	char *			outBufPtr;
	CrossClass::cMutex	outBufMutex;
	
public:
	virtual bool handle_read ();		/* return true if a message was received	*/
	virtual bool handle_write ();		/* return true if a send should be pending*/
	
	virtual crs_fd_t get_descriptor ();
	
	virtual bool needs_prepare ();	/* needs preprocessing before polling	*/
	virtual bool want2write ();		/* transmission is pending			*/
	virtual bool auto_destroy ();		/* destroy object by 'event_loop' class	*/
	
public:
	void open ( const std::string & portName = "/dev/ttyS0", const size_t baudRate = 115200 );
	void open ( std::istream & );
	
	virtual void close ();
	virtual void write ( const char * lpBuf, const size_t dwToWrite );
	/* NOTE: write could leave message pending */
	
	bool post ( const char * lpBuf, const size_t dwToWrite );
	/* return true if data was sucessfully placed into the out queue	*/
	/* and false otherwise								*/
	
	posixRS232port ( const size_t inBufSize = 2048 );
	virtual ~posixRS232port ();
};

} // namespace sc
#endif // CROSS_SC_POSIXRS232PORT_H_INCLUDED
