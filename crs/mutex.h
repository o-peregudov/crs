#ifndef CROSS_MUTEX_H_INCLUDED
#define CROSS_MUTEX_H_INCLUDED 1
// (c) Jan 22, 2008 Oleg N. Peregudov
// Aug 26, 2010 - separate versions for each API
// Aug 28, 2010 - compartibility with std::mutex from C++0x standard

#include <crs/libexport.h>

#if defined( USE_WIN32_API )
#	include <crs/bits/win32.mutex.h>
#elif defined( USE_CXX0X_API )
#	include <mutex>
#elif defined( USE_POSIX_API )
#	include <crs/bits/posix.mutex.h>
#endif

namespace CrossClass
{
#if defined( USE_WIN32_API )
	typedef cWin32Mutex	cHostMutexType;
#	define DO_DEFINE_CMUTEX_CLASS 1
#elif defined( USE_CXX0X_API )
	typedef std::mutex	cMutex;
#	define CAN_USE_STD_UNIQUE_LOCK 1
#elif defined( USE_POSIX_API )
	typedef cPosixMutex	cHostMutexType;
#	define DO_DEFINE_CMUTEX_CLASS 1
#endif
} // namespace CrossClass

#if defined( DO_DEFINE_CMUTEX_CLASS )
namespace CrossClass
{
	class CROSS_EXPORT cMutex : public cHostMutexType
	{
	public:
		typedef cHostMutexType::native_handle_type native_handle_type;
		
		cMutex () : cHostMutexType( )	{ }
		~cMutex ()				{ }
		
		operator native_handle_type & ()
		{
			return cHostMutexType::operator cHostMutexType::native_handle_type & ();
		}
		
		operator const native_handle_type & () const
		{
			return cHostMutexType::operator const cHostMutexType::native_handle_type & ();
		}
		
		operator native_handle_type * ()
		{
			return cHostMutexType::operator cHostMutexType::native_handle_type * ();
		}
		
		operator const native_handle_type * () const
		{
			return cHostMutexType::operator const cHostMutexType::native_handle_type * ();
		}
	};
} // namespace CrossClass
#undef DO_DEFINE_CMUTEX_CLASS
#endif
#endif // CROSS_MUTEX_H_INCLUDED

