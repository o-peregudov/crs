#include <crs/thread.h>
#include <crs/events.h>
#include <vector>

class myThreadA : public CrossClass::cThread
{
protected:
	CrossClass::cEvent _evnt;
	
	std::vector<double> * pv;
	size_t idxStart,
		 idxStride,
		 n;
	bool	 done;
	
	virtual void * StepResult ( const int errCode )
	{
		int a;
		printf( "StepResult? " );
		scanf( "%d", &a );
		return reinterpret_cast<void *>( a );
	}
	
	virtual bool Step ( )
	{
		printf( "thread hit .... " );
		Stop();
		for( size_t i = idxStart; i < n; i += idxStride )
			(*pv)[ i ] = i * i;
		printf( "signal!\n" );
		_evnt.SetEvent();
		return false;
	}
	
public:
	myThreadA ()
		: CrossClass::cThread( false )
		, _evnt( false, false )
		, pv( 0 )
		, idxStart( 1 )
		, idxStride( 2 )
		, n( 0 )
		, done( false )
	{
	}
	
	~myThreadA ()
	{
		printf( "myThreadA::~myThreadA\n" );
		kill();
	}
	
	void	restart ( std::vector<double> * v, const size_t nCount );
	void	wait ();
};

void	myThreadA::restart ( std::vector<double> * v, const size_t nCount )
{
	n = nCount;
	pv = v;
	printf( "restarted!\n" );
	Resume();
}

void	myThreadA::wait ()
{
	printf( "waiting!\n" );
	_evnt.WaitEvent();
}

int main ()
{
	myThreadA threadA;
	std::vector<double> v ( 10000000 );
	for( size_t i = 0; i < 100; ++i )
	{
		printf( "Start cycle %d - ", i );
		threadA.restart( &v, v.size() );
		for( size_t j = 0; j < v.size(); j += 2 )
			v[ j ] = j * j;
		threadA.wait();
		printf( "Cycle %d - ", i );
		for( size_t k = 0; k < v.size(); ++k )
		{
			if( v[ k ] != ( k * k ) )
			{
				printf( "Fail with element #%d\n", k );
				return 1;
			}
			v[ k ] = -1;
		}
		printf( "ok!\n" );
	}
	return 0;
}
