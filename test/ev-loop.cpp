#if defined( HAVE_CONFIG_H )
#	include "config.h"
#endif

#include <cstdlib>
#include <csignal>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <crs/eveloop.h>
#include <crs/ipchannel.h>
#include <crs/thread.h>
#include <crs/atomic_flag.h>

/******************************************************************************/
/*													*/
/* struct callback_sink										*/
/*													*/
/******************************************************************************/
struct callback_sink
{
	std::vector<size_t>	vec;
	CrossClass::cMutex	mutex;
	
	callback_sink ()
		: vec (('Z' - 'A') + 1, 0)
		, mutex ()
	{ }
	
	static void incoming_callback (CrossClass::basic_ip_channel::incoming_message_callback_data * cb_data)
	{
		callback_sink * cb_sink = reinterpret_cast<callback_sink *> (cb_data->data);
		cb_sink->process_unlocked (cb_data->message, cb_data->size);
	}
	
	void process_unlocked (const char * buf, const size_t sz)
	{
		for (size_t ix = 0; ix < sz; ++ix)
			++(vec[ buf[ ix ] - 'A' ]);
	}
	
	void process_locked (const char * buf, const size_t sz)
	{
		CrossClass::_LockIt process_lock (mutex);
		process_unlocked (buf, sz);
	}
};

std::ostream & operator << (std::ostream & os, const callback_sink & snk)
{
	for (size_t ix = 0; ix < snk.vec.size (); ++ix)
	{
		os	<< static_cast<const char> ('A' + ix)
			<< '\t'
			<< snk.vec[ ix ]
			<< std::endl;
	}
	return os;
}

/******************************************************************************/
/*													*/
/* class channel_data										*/
/*													*/
/******************************************************************************/
struct channel_data
{
	CrossClass::ip_channel::incoming_message_callback_data cb_data;
	CrossClass::ip_channel channel;
	std::string string;
	size_t ix;
	size_t iy;
	
	channel_data ( CrossClass::event_loop * elp, callback_sink * snk, std::string & str )
		: cb_data (512, snk)
		, channel (elp, callback_sink::incoming_callback, &cb_data)
		, string (str)
		, ix (10)
		, iy (0)
	{
		std::random_shuffle (string.begin(), string.end());
	}
	
	bool hit ()
	{
		channel.push ((string.c_str () + iy), 1);
		if (++iy == string.size ())
		{
			iy = 0;
			if (--ix == 0)
				return true;
		}
		return false;
	}
};

static CrossClass::event_loop * evpoll4signal = 0;

static void terminate ( int signal )
{
	evpoll4signal->terminate (false);
}

int	main ( int argc, const char ** argv )
{
	if (argc > 1)
	{
		char ch = 'n';
		std::cout << "Are you ready (y/n)? ";
		std::cin.get (ch);
		if (ch != 'y')
			return 0;
	}
	
	try
	{
		std::cout	<< "Creating event loop ..." << std::flush;
		CrossClass::event_loop eveloop (1);
		std::cout	<< " done"<< std::endl;
		
//		std::cout	<< "Configuring SIGINT handler ..." << std::flush;
//		evpoll4signal = &eveloop;
//		signal (SIGINT, terminate);
//		std::cout	<< " done" << std::endl;
		
		callback_sink sink;
		std::string str =
			"AAAAAAAAAAAAAAABBBBBBBBBBBBBBBCCCCCCCCCCCCCCCDDDDDDDDDDDDDDD"
			"EEEEEEEEEEEEEEEFFFFFFFFFFFFFFFGGGGGGGGGGGGGGGHHHHHHHHHHHHHHH"
			"IIIIIIIIIIIIIIIJJJJJJJJJJJJJJJKKKKKKKKKKKKKKKLLLLLLLLLLLLLLL"
			"MMMMMMMMMMMMMMMNNNNNNNNNNNNNNNOOOOOOOOOOOOOOOPPPPPPPPPPPPPPP"
			"QQQQQQQQQQQQQQQRRRRRRRRRRRRRRRSSSSSSSSSSSSSSSTTTTTTTTTTTTTTT"
			"UUUUUUUUUUUUUUUVVVVVVVVVVVVVVVWWWWWWWWWWWWWWWXXXXXXXXXXXXXXX"
			"YYYYYYYYYYYYYYYZZZZZZZZZZZZZZZ";
		
		std::vector<channel_data *> thrd;
		
		#pragma omp parallel sections
		{
			#pragma omp section
			for (;;)
			{
				if (thrd.empty ())
				{
					thrd.push_back (new channel_data (&eveloop, &sink, str));
					std::cout << "New channel #" << thrd.size () << std::endl;
				}
				if (thrd.back ()->hit ())
				{
					if (thrd.size () == 5)
					{
						eveloop.terminate ();
						break;
					}
					else
					{
						thrd.push_back (new channel_data (&eveloop, &sink, str));
						std::cout << "New channel #" << thrd.size () << std::endl;
					}
				}
			}
			
			#pragma omp section
			while (eveloop.runOnce (-1));
		}
//		for (eveloop.restart (); eveloop.runOnce (-1); )
//		{
//			if (thrd.empty ())
//				thrd.push_back (new channel_data (&eveloop, &sink, str));
//			if (thrd.back ()->hit ())
//			{
//				if (thrd.size () == 10)
//					eveloop.terminate ();
//				else
//					thrd.push_back (new channel_data (&eveloop, &sink, str));
//			}
//		}
		
		for (size_t ix = 0; ix < thrd.size (); ++ix)
		{
			std::cout << "Release channel #" << (ix + 1) << std::endl;
			delete thrd[ ix ];
			thrd[ ix ] = 0;
		}
		
		std::cout << std::endl << sink << std::endl;
	}
	catch (std::runtime_error & e)
	{
		std::cerr
			<< "Runtime exception: " << e.what () << std::endl;
	}
	
	return 0;
}
