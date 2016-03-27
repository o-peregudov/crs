#ifndef CROSS_BARRIER_H_INCLUDED
#define CROSS_BARRIER_H_INCLUDED 1
//
// barrier.h - interface of the class cBarrier
// (c) Feb 20, 2009 Oleg N. Peregudov
//

#if defined( __GNUG__ )
#	include <pthread.h>
#elif defined( _MSC_VER )
#endif
#include <stdexcept>
#include "cross/libexport.h"

namespace CrossClass
{

class CROSS_EXPORT cBarrier
{
#if defined( __GNUG__ )
protected:
	pthread_barrier_t	_barrier;
	
#elif defined( _MSC_VER )
	
#endif
	
public:
	cBarrier ( const unsigned count );
	~cBarrier ();
	
	bool operator () ();
};

} // namespace CrossClass
#endif // CROSS_BARRIER_H_INCLUDED
