#include <crs/mtsqueue.h>
#include <iostream>
#include <windows.h>

struct data_type
{
	double a, b, c, d, e, f;
	size_t n;
};

typedef CrossClass::cMTSQueue<data_type> queue;

CrossClass::LockType theLock;
size_t globalCounter ( 0 );

DWORD WINAPI ThreadFunc ( LPVOID lpParam )
{
	queue * q = reinterpret_cast<queue *>( lpParam );
	CrossClass::_LockIt ea ( theLock );
	for( data_type a;; )
	{
		ea.Lock();
		a.n = ++globalCounter;
		std::cout << "<- " << a.n << ' ' << std::endl;
		ea.Unlock();
		q->push( a );
		Sleep( 1 );
	}
	return 0;
}

int main ()
{
	queue q;
	data_type a;
	
	CreateThread(
      	NULL,             // default security attributes 
            0,                // use default stack size  
            ThreadFunc,       // thread function 
            &q,             	// argument to thread function 
            0,                // use default creation flags 
            0 );    		// returns the thread identifier
	
	CreateThread(
      	NULL,             // default security attributes 
            0,                // use default stack size  
            ThreadFunc,       // thread function 
            &q,             	// argument to thread function 
            0,                // use default creation flags 
            0 );    		// returns the thread identifier
	
	CreateThread(
      	NULL,             // default security attributes 
            0,                // use default stack size  
            ThreadFunc,       // thread function 
            &q,             	// argument to thread function 
            0,                // use default creation flags 
            0 );    		// returns the thread identifier
	
	CreateThread(
      	NULL,             // default security attributes 
            0,                // use default stack size  
            ThreadFunc,       // thread function 
            &q,             	// argument to thread function 
            0,                // use default creation flags 
            0 );    		// returns the thread identifier
	
	CreateThread(
      	NULL,             // default security attributes 
            0,                // use default stack size  
            ThreadFunc,       // thread function 
            &q,             	// argument to thread function 
            0,                // use default creation flags 
            0 );    		// returns the thread identifier
	
	for( ;; )
	{
		if( !q.empty() )
		{
			a = q.front();
			q.pop();
			CrossClass::_LockIt ea ( theLock, true );
			std::cout << "-> " << a.n << std::endl;
		}
	}
	return 0;
}
