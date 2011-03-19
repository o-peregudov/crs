//////////////////////////////////////////////////////////////////////
//
// basic.rs232port.cpp: bits of the implementation of the rs232port class.
// (c) Aug 31, 2010 Oleg N. Peregudov
//
//////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )
#	pragma warning( disable : 4251 )
#	pragma warning( disable : 4275 )
#	pragma warning( disable : 4996 )
#endif
#include <crs/sc/basic.rs232port.h>
#include <sstream>
#include <cerrno>
#include <iomanip>

namespace sc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

basicRS232port::basicRS232port( const size_t inBufSize )
	: m_cComPortName()
	, m_Parity( 0 )
	, m_bConnected( false )
	, m_dwTimeOut( 1500 )
	//
	// incoming buffer members
	//
	, _inBufLock( )
	, _inBufSize( inBufSize )
	, _inBuf( 0 )
	, _inBufPtr( 0 )
	, _strInBuf( )
	, _callBackFunc( 0 )
	, _callBackData( 0 )
	, _callBackLock( )
{
	_inBuf = _inBufPtr = new char [ _inBufSize ];
}

basicRS232port::~basicRS232port()
{
	delete [] _inBuf;
}

bool basicRS232port::synchronize ( const unsigned long dwTimeOut )
{
	return true;
}

void basicRS232port::incomingData ()
{
	CrossClass::_LockIt exclusive_access ( _callBackLock );
	if( _callBackFunc )
		_callBackFunc( _callBackData );
}

void basicRS232port::inBufAppend ( const char * pData, const size_t nData )
{
	CrossClass::_LockIt exclusive_access ( _callBackLock );
	_strInBuf.append( pData, nData );
}

void basicRS232port::processIncomingBytes ( const size_t dwRead )
{
	_inBufPtr += dwRead;
	size_t nBytesAvailable = _inBufPtr - _inBuf,
		nBytesProcessed = compileResponse( _inBuf, nBytesAvailable );
	if( nBytesProcessed )
	{
		size_t nBytesRest = nBytesAvailable - nBytesProcessed;
		if( nBytesRest )
			memcpy( _inBuf, _inBuf + nBytesProcessed, nBytesRest );
		_inBufPtr = _inBuf;
	}
}

size_t basicRS232port::compileResponse ( const char * lpBuf, const size_t dwAvailable )
{
	static const char * delim = "\n\r";
	const char * p = strpbrk( lpBuf, delim );
	if( p )
	{
		size_t nBytes2Save = p - lpBuf;
		if( nBytes2Save < dwAvailable )
		{
			inBufAppend( lpBuf, nBytes2Save );
			incomingData();
			while( ( nBytes2Save < dwAvailable ) && ( ( *p == '\n' ) || ( *p == '\r' ) ) )
				++p, ++nBytes2Save;
			return nBytes2Save;
		}
	}
	return 0;
}

std::string basicRS232port::getString ()
{
	CrossClass::_LockIt exclusive_access ( _inBufLock );
	std::string result = _strInBuf;
	_strInBuf = "";
	return result;
}

void basicRS232port::setAsyncDataCallback ( asyncDataCallBackFunction func, void * pData )
{
	CrossClass::_LockIt exclusive_access ( _callBackLock );
	_callBackFunc = func;
	_callBackData = pData;
}

bool basicRS232port::receive ()
{
	return false;	// terminate thread
}

void basicRS232port::swrite ( const std::string & str )
{
	write( str.c_str(), str.size() );
}

} // namespace sc
