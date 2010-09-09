// (c) Aug 30, 2010 Oleg N. Peregudov
#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#endif
#include <crs/bits/win32.cond.h>

namespace CrossClass {

//
// members of class cWin32ConditionVariable
//
cWin32ConditionVariable::cWin32ConditionVariable ()
	: _event( CreateEvent( NULL, FALSE, FALSE, NULL ) )
{
	if( _event == NULL )
	{
		char msgText [ 64 ];
		sprintf( msgText, "CreateEvent returned 0x%X", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

cWin32ConditionVariable::~cWin32ConditionVariable ()
{
	if( CloseHandle( _event ) == 0 )
	{
		char msgText [ 64 ];
		sprintf( msgText, "CloseHandle returned 0x%X", GetLastError() );
		throw std::runtime_error( msgText );
	}
}

} // namespace CrossClass
