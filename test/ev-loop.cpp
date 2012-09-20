#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <crs/eveloop.h>
#include <cstdlib>
#include <csignal>
#include <iostream>

CrossClass::event_loop * evpoll4signal = 0;

void	terminate ( int signal )
{
	//std::cout << "\rPress Ctrl+C to terminate ... ";
	evpoll4signal->terminate (false);
	//std::cout << "pressed" << std::endl;
}

int	main ( int argc, const char ** argv )
{
	try
	{
		std::cout
			<< "Creating event loop ..."
			<< std::flush;
		
		CrossClass::event_loop eveloop (1);
		
		std::cout
			<< " done"
			<< std::endl;
		
		std::cout
			<< "Configuring SIGINT handler ..."
			<< std::flush;
		
		evpoll4signal = &eveloop;
		signal (SIGINT, terminate);
		
		std::cout
			<< " done"
			<< std::endl;
		
		std::cout
			<< "Entering main loop. Press Ctrl+C to terminate ... "
			<< std::flush;
		
		while (eveloop.runOnce (-1));
	}
	catch (std::runtime_error & e)
	{
		std::cerr
			<< "Runtime exception: " << e.what () << std::endl;
	}
	
	return 0;
}
