#include <iostream>
#include <crs/thread.h>
#include <crs/timer.h>
#include "rrtg.2/ms-data-core/composite_core.h"

#define NCHANNELS		2
MSDataCore::cDAQCore	rp ( NCHANNELS );
CrossClass::cTimer	clk;

class insertion_thread : public CrossClass::cThread
{
protected:
	virtual bool Step ()
	{
		MSDataCore::cIntensityPoint point ( NCHANNELS );
		point->time = clk();
		for( int i = 0; i < point.size(); ++i )
			point->intensity[ i ] = -(point->time + i);
		rp.acquire( point );
		CrossClass::sleep( 1 );
		return false;
	}
public:
	insertion_thread ()
		: CrossClass::cThread( )
	{ }
	~insertion_thread ()
	{
		kill();
	}
};

int main ( )
{
	double startTime = clk();
	
	insertion_thread trd;
	trd.Resume();
	
	MSDataCore::cBuildPoint ip;
	MSDataCore::cDAQCore::time_iter it = rp.begin(), en;
	while( ( clk() - startTime ) < 1200.0 )
	{
		en = rp.end();
		for( ; it != en; ++it )
		{
			ip = *it;
			std::cout << ip->time << '\r' << std::flush;
		}
		CrossClass::sleep( 1 );
	}
	
	return 0;
}

