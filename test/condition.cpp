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
	data	*d;
	int	nreps;
	
	virtual bool Step ()
	{
		if (++nreps == NREPS)
		{
			Stop ();	/* suspend thread		*/
			nreps = 0;	/* and reset counter	*/
		}
		
		{
			CrossClass::_LockIt lockIn ( d->_condInMutex );
			while (!d->_flagIn)
				d->_condIn.wait (lockIn);
			d->_flagIn = false;
		}
		
		std::cout << ')' << std::flush;
		
		{
			CrossClass::_LockIt lockOut ( d->_condOutMutex );
			d->_flagOut = true;
			d->_condOut.notify_one ();
		}
		
		return false;
	}
	
public:
	thread ( data * p )
		: CrossClass::cThread( false )
		, d( p )
		, nreps( 0 )
	{ }
	
	virtual ~thread ()
	{
		std::cout << "ClientThread destructor: going to kill the thread ... " << std::flush;
		kill ();
		std::cout << "done" << std::endl;
	}
};

struct notBool
{
	bool & flag;
	notBool ( bool & f ) : flag (f) {}
	bool operator () () { return flag; };
};

int main ()
{
	data D;
	D._flagIn = false;
	D._flagOut = false;
	
	thread T (&D);
	for (int ix = 0; ix < (NREPS<<3); ++ix)
	{
		std::cout	<< "Pass " << ix
				<< ", thread status: "
				<< "active = " << std::boolalpha << T.active ()
				<< ", running = " << std::boolalpha << static_cast<bool> (T) << std::endl
				<< '\t' << std::flush;
		
		T.Resume ();
		
		for (int iy = 0; iy < NREPS; ++iy)
		{
			std::cout << '(' << (iy + 1) << std::flush;
			
			{
				/* push the thread */
				CrossClass::_LockIt lockIn ( D._condInMutex );
				D._flagIn = true;
				D._condIn.notify_one ();
			}
			
			{
				/* waiting for response */
				CrossClass::_LockIt lockOut ( D._condOutMutex );
				while (!D._flagOut)
					D._condOut.wait (lockOut);
				D._flagOut = false;
			}
		}
		
		std::cout << std::endl;
	}
	
	std::cout << "Final thread status: "
		<< "active = " << std::boolalpha << T.active ()
		<< ", running = " << std::boolalpha << static_cast<bool> (T) << std::endl;
	
	std::cout << "Waiting for 'out'-condition ... " << std::flush;
	CrossClass::_LockIt lockOut ( D._condOutMutex );
	D._flagOut = false;
	if (D._condOut.wait_for (lockOut, 250, notBool (D._flagOut)))
		std::cout << "success" << std::endl;
	else
		std::cout << "timeout" << std::endl;
	
	return 0;
}
