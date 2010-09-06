// (c) Jan 31, 2009 Oleg N. Peregudov
// Apr 23, 2009 - Win/Posix defines
// Aug 24, 2010 - new server termination algorithm based on events
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#endif
#include <crs/bits/win32.netpoint.h>
#include <sstream>

namespace CrossClass {

//
// members of class win32NetPoint
//
win32NetPoint::win32NetPoint ()
	: basicNetPoint( )
	, evntTerminate( CreateEvent( NULL, TRUE, FALSE, NULL ) )
{
}

win32NetPoint::win32NetPoint ( cSocket & clientSocket )
	: basicNetPoint( clientSocket )
	, evntTerminate( CreateEvent( NULL, TRUE, FALSE, NULL ) )
{
	setNonBlock();
}

win32NetPoint::~win32NetPoint ()
{
	shutdown( _socket, SD_BOTH );
	CloseHandle( evntTerminate );
}

void win32NetPoint::setNonBlock ()
{
	// set socket to non-blocking
	
}

timeval * win32NetPoint::onStartServer ()
{
	ResetEvent( evntTerminate );
	return &selectTimeOut;
}

bool win32NetPoint::preCheckTerminate ()
{
	if( WaitForSingleObject( evntTerminate, 0 ) == WAIT_OBJECT_0 )
		return true;
	selectTimeOut.milliseconds( 5 );
	return false;
}

cHandle<basicNetPoint> win32NetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return new win32NetPoint( s );
}

void win32NetPoint::terminateServer ()
{
	SetEvent( evntTerminate );
}

} // namespace CrossClass

