#ifndef CROSS_WIN32_LOCK_H_INCLUDED
#define CROSS_WIN32_LOCK_H_INCLUDED 1
// (c) Jan 22, 2008 Oleg N. Peregudov
// Aug 24, 2010 - Win32 mutex envelope
// Aug 27, 2010 - compartibility with std::mutex from C++0x standard

#include <crs/libexport.h>
#include <windows.h>
#include <stdexcept>

namespace CrossClass
{
	class CROSS_EXPORT cWin32Mutex
	{
	protected:
		HANDLE _mutex;
	
	public:
		typedef HANDLE native_handle_type;
		
		cWin32Mutex ();
		~cWin32Mutex ();
		
		void lock ();
		bool try_lock ();
		void unlock ();
		
		operator native_handle_type & ()			{ return _mutex; }
		operator const native_handle_type & () const	{ return _mutex; }
		
		operator native_handle_type * ()			{ return &_mutex; }
		operator const native_handle_type * ()	const { return &_mutex; }
	};
} // namespace CrossClass
#endif // CROSS_WIN32_LOCK_H_INCLUDED
