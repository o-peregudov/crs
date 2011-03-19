#ifndef CROSS_SC_WIN32RS232PORT_H_INCLUDED
#define CROSS_SC_WIN32RS232PORT_H_INCLUDED 1
//////////////////////////////////////////////////////////////////////
//
// win32.rs232port.h: interface for the rs232port class (Win32 API).
// (c) Sep 4, 2010 Oleg N. Peregudov
//	09/09/2010	baudrate constant selector
//	09/20/2010	non-blocking postTerminate
//
//////////////////////////////////////////////////////////////////////

#include <crs/sc/basic.rs232port.h>
#include <windows.h>

namespace sc {

//
// class win32RS232port
//
class CROSS_EXPORT win32RS232port : public basicRS232port
{
protected:
	HANDLE		m_hCommPort;
	COMMTIMEOUTS	m_TimeoutsOrig;
	COMMTIMEOUTS	m_TimeoutsNew;
	HANDLE		m_evntRead,
				m_evntWrite,
				m_evntTerminate,
				m_evntTerminated;
	
	virtual void UpdateConnection ();
	
public:
	void open ( const std::string & portName = "COM1", const size_t baudRate = 115200 );
	
	virtual void close ();
	virtual void write ( const char * lpBuf, const size_t dwToWrite );
	virtual bool receive ();
	virtual void postTerminate ( const bool doWaitTerminate = true );
	
	win32RS232port ( const size_t inBufSize = 2048 );
	virtual ~win32RS232port();
};

} // namespace sc
#endif // CROSS_SC_WIN32RS232PORT_H_INCLUDED
