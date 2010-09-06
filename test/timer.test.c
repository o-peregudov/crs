#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

int main ()
{
	unsigned long msDuration = 10000;
	
	if( msDuration == 0 )
		sched_yield();
	else
	{
		int res;
		struct timespec rqtp, rmtp;
		rqtp.tv_sec = msDuration / 1000;
		rqtp.tv_nsec = ( msDuration % 1000 ) * 1000000;
		do
		{
			res = nanosleep( &rqtp, &rmtp );
			if( res == -1 )
				rqtp = rmtp;
		}
		while( res == -1 ) ;
	}
	
	/*
	struct timespec abstime;
	
	int i, j;
	struct timespec rqtp, rmtp;
	clock_getres( CLOCK_REALTIME, &abstime );
	printf( "Resolution %ld ; %ld\n", abstime.tv_sec, abstime.tv_nsec );
	
	clock_gettime( CLOCK_REALTIME, &abstime );
	memcpy( &rqtp, &abstime, sizeof( struct timespec ) );
	for( i = 0; i < 1000; ++i )
	{
		for( j = 0; j < 1000000; ++j );
		clock_gettime( CLOCK_REALTIME, &abstime );
		printf( "%ld ; %ld ; %ld\n", abstime.tv_sec, abstime.tv_nsec, abstime.tv_nsec - rqtp.tv_nsec );
		memcpy( &rqtp, &abstime, sizeof( struct timespec ) );
	}
	*/
	return 0;
}
