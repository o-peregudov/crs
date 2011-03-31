#include <crs/thread.h>
#include <iostream>
#include <iomanip>

class sampleThread : public CrossClass::cThread
{
protected:
	virtual bool Step ( )
	{
		std::cerr << "." << std::flush;
		CrossClass::sleep( 150 );
		return false;
	}
	
public:
	sampleThread ()
		: CrossClass::cThread( false )
	{
	}
	
	~sampleThread ()
	{
		kill();
	}
};

int main ()
{
	sampleThread thrd;
	std::cout << "Step 0: active = " << std::boolalpha << thrd.active() << std::endl;
	thrd.Resume();
	std::cout << "Step 1: active = " << std::boolalpha << thrd.active() << std::endl;
	CrossClass::sleep( 3000 );
	thrd.Stop();
	std::cout << "Step 2: active = " << std::boolalpha << thrd.active() << std::endl;
	CrossClass::sleep( 3000 );
	thrd.Resume();
	std::cout << "Step 3: active = " << std::boolalpha << thrd.active() << std::endl;
	CrossClass::sleep( 3000 );
	return 0;
}
