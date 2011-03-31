#include <crs/thread.h>
#include <crs/condition_variable.hpp>
#include <iostream>
#include <iomanip>

#define NREPS	25

struct data
{
	CrossClass::cMutex			_condInMutex,
							_condOutMutex;
	CrossClass::cConditionVariable	_condIn,
							_condOut;
	bool						_flagIn,
							_flagOut;
};

class thread : public CrossClass::cThread
{
protected:
	data * d;
	
	virtual bool Step ()
	{
		{
			CrossClass::_LockIt lockIn ( d->_condInMutex );
			while( !d->_flagIn )
				d->_condIn.wait( lockIn );
			d->_flagIn = false;
		}
		std::cout << "pong" << std::endl;
		{
			CrossClass::_LockIt lockOut ( d->_condOutMutex );
			d->_flagOut = true;
			d->_condOut.notify_one();
		}
		return false;
	}
	
public:
	thread ( data * p )
		: CrossClass::cThread( false )
		, d( p )
	{ }
	
	virtual ~thread ()
	{
		std::cout << "going to kill the thread ... " << std::flush;
		kill();
		std::cout << "done" << std::endl;
	}
};

int main ()
{
	data D;
	D._flagIn = D._flagOut = false;
	thread T( &D );
	T.Resume();
	for( int i = 0; i < NREPS; ++i )
	{
		if( i == ( NREPS - 1 ) )
			T.Stop();
		std::cout << ( i + 1 ) << ": ping ... " << std::flush;
		{
			CrossClass::_LockIt lockIn ( D._condInMutex );
			D._flagIn = true;
			D._condIn.notify_one();
		}
		{
			CrossClass::_LockIt lockOut ( D._condOutMutex );
			while( !D._flagOut )
				D._condOut.wait( lockOut );
			D._flagOut = false;
		}
	}
	std::cout << "thread status:" << std::endl
		<< "\tactive: " << std::boolalpha << T.active() << std::endl
		<< "\trunning: " << std::boolalpha << static_cast<bool>( T ) << std::endl;
	return 0;
}
