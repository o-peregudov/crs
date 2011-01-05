#include <crs/thread.h>
#include <crs/condition_variable.h>
#include <iostream>

struct data
{
	CrossClass::LockType			_condInMutex,
							_condOutMutex;
	CrossClass::cConditionVariable	_condIn,
							_condOut;
	bool						_flagIn,
							_flagOut;
};

struct directPredicate
{
	bool * flag;
	directPredicate ( bool * f ) : flag( f ) {}
	bool operator () ( ) { return *flag; }
};

class thread : public CrossClass::cThread
{
protected:
	data * d;
	
	virtual bool Step ()
	{
		try
		{
			{
				CrossClass::_LockIt lockIn ( d->_condInMutex );
				for( d->_flagIn = false; !d->_flagIn; d->_condIn.wait( lockIn ) );
			}
			{
				CrossClass::_LockIt lockOut ( d->_condOutMutex );
				d->_flagOut = true;
				d->_condOut.notify_one();
			}
			std::cout << "pong!" << std::endl;
		}
		catch( std::runtime_error & e )
		{
			std::cerr << "Error: " << e.what () << std::endl;
		}
		return false;
	}
	
public:
	thread ( data * p ) : CrossClass::cThread( false ), d( p ) {}
	virtual ~thread ()
	{
		kill();
	}
};

int main ()
{
	data D;
	D._flagIn = D._flagOut = false;
	
	int i;
	std::cout << "? ";
	std::cin >> i;
	
	thread T( &D );
	T.Resume();
	
	std::cout << "? ";
	std::cin >> i;
	
	for( i = 0; i < 1000; ++i )
	{
		try
		{
			{
				CrossClass::_LockIt lockIn ( D._condInMutex );
				D._flagIn = true;
				D._condIn.notify_one();
			}
			std::cout << i << ": ping!" << std::flush;
			{
				CrossClass::_LockIt lockOut ( D._condOutMutex );
				for( D._flagOut = false; !D._flagOut; D._condOut.wait( lockOut ) );
			}
		}
		catch( std::runtime_error & e )
		{
			std::cerr << "Error: " << e.what () << std::endl;
		}
	}
	{
		CrossClass::_LockIt lockIn ( D._condInMutex );
		D._flagIn = true;
		D._condIn.notify_one();
	}
	T.Stop();
	
	return 0;
}

