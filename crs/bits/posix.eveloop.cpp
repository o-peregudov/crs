/*
 *  crs/bits/posix.eveloop.cpp
 *  Copyright (c) 2012 Oleg N. Peregudov <o.peregudov@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined (HAVE_CONFIG_H)
#	include "config.h"
#endif

#if defined (EPOLL_IO_MULTIPLEXING)
#	include <sys/epoll.h>
#elif defined (POLL_IO_MULTIPLEXING)
#	include <poll.h>
#elif defined (SELECT_IO_MULTIPLEXING)
#	include <sys/select.h>
#endif
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <crs/eveloop.h>
#include <crs/security.h>
#include <crs/atomic_flag.h>

namespace CrossClass {

struct poll_records
{
	int						control_pipe [ 2 ];
	char						control_buffer [ 256 ];
	cAtomicBool					restart_flag;
#if defined (EPOLL_IO_MULTIPLEXING)
	int epollfd;
	std::vector<event_descriptor *>	prepare_list;
	std::vector<epoll_event>		events;
#elif defined (POLL_IO_MULTIPLEXING)
	std::vector<pollfd>			events;
#elif defined (SELECT_IO_MULTIPLEXING)
#endif
	
	poll_records ( const size_t ExpectedNumberOfDescriptors )
		: control_pipe ()
		, control_buffer ()
#if defined (EPOLL_IO_MULTIPLEXING)
		, epollfd (-1)
		, prepare_list ()
#elif defined (POLL_IO_MULTIPLEXING)
#elif defined (SELECT_IO_MULTIPLEXING)
#endif
		, events ()
	{
		/*
		 * create an interprocess communication pipe for the read end
		 */
		if (pipe (control_pipe) == -1)
		{
			char msgText [ 256 ];
			sprintf (msgText, "%d: '%s' in pipe", errno, strerror (errno));
			throw event_loop::errCreate (msgText);
		}
		
		/*
		 * allocate memory for the descriptors lists
		 */
		events.reserve (ExpectedNumberOfDescriptors + 1);
		
#if defined (EPOLL_IO_MULTIPLEXING)
		/*
		 * with epoll not all descriptors have to be scanned in every iteration
		 */
		prepare_list.reserve (ExpectedNumberOfDescriptors + 1);
#endif
	}
	
	~poll_records ()
	{
		/*
		 * close termination pipe
		 */
		int errCode = close (control_pipe[ 1 ]);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
		if (errCode == -1)
		{
			char msgText [ 256 ];
			sprintf (msgText, "%d: '%s' in close(1)", errno, strerror (errno));
			throw event_loop::errClose (msgText);
		}
#endif
		errCode = close (control_pipe[ 0 ]);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
		if (errCode == -1)
		{
			char msgText [ 256 ];
			sprintf (msgText, "%d: '%s' in close(0)", errno, strerror (errno));
			throw event_loop::errClose (msgText);
		}
#endif
	}
	
	bool check_terminate ()
	{
		const ssize_t nBufSize = sizeof (control_buffer) / sizeof (char);
		for (ssize_t nBytesRead = 0;;)
		{
			nBytesRead = read (control_pipe[ 0 ], control_buffer, nBufSize);
			if (nBytesRead == -1)
			{
				if (errno == EINTR)
					continue;
				else if (errno == EAGAIN)
					break;
				else
				{
					char msgText [ 256 ];
					sprintf (msgText, "%d: '%s' in read", errno, strerror (errno));
					throw event_loop::errControl (msgText);
				}
			}
			else if ((nBytesRead == 0) || (memchr (control_buffer, 'T', nBytesRead)))
				return true;
			else if (nBytesRead < nBufSize)
				break;
		}
		return false;
	}
	
	void pipe_out ( const char c )
	{
		for (ssize_t nBytesWritten = 0;;)
		{
			nBytesWritten = write (control_pipe[ 1 ], &c, 1);
			if (nBytesWritten == -1)
			{
				if (errno == EINTR)
					continue;
				else
				{
					char msgText [ 256 ];
					sprintf (msgText, "%d: '%s' in write", errno, strerror (errno));
					throw event_loop::errControl (msgText);
				}
			}
			else if (nBytesWritten == 1)
				break;
		}
	}
};

#define NUMBER_OF_STANDARD_DESCRIPTORS	3
/* standard descriptors are {stdin, stdout, stderr} */

event_loop::event_loop ( const size_t ExpectedNumberOfDescriptors )
	: descriptors_list ()
	, descriptors_list_mutex ()
	, terminate_mutex ()
	, terminate_condition ()
	, terminate_flag ( false )
	, records ( 0 )
{
	/*
	 * allocate memory for descriptors lists
	 */
	descriptors_list.reserve (ExpectedNumberOfDescriptors + NUMBER_OF_STANDARD_DESCRIPTORS + 1);
	poll_records * recs = new poll_records ( ExpectedNumberOfDescriptors );
	records = recs;
	
#if defined (EPOLL_IO_MULTIPLEXING)
	/*
	 * create epoll file descriptor
	 */
	recs->epollfd = epoll_create (ExpectedNumberOfDescriptors + 1);
	if (recs->epollfd == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in epoll_create", errno, strerror (errno));
		throw errCreate (msgText);
	}
	
	/*
	 * add read pipe end descriptor into the epoll descriptors list
	 */
	epoll_event ev;
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = recs->control_pipe[ 0 ]; /* read end */
	if (epoll_ctl (recs->epollfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in EPOLL_CTL_ADD", errno, strerror (errno));
		throw errControl (msgText);
	}
	
	/*
	 * track the number of descriptors
	 * actually we'll need this structure for epoll_wait
	 */
	recs->events.push_back (ev);
#endif
}

event_loop::~event_loop ()
{
	int errCode = 0;
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	
#if defined (EPOLL_IO_MULTIPLEXING)
	/*
	 * remove read pipe end from the epoll descriptors list
	 */
	epoll_event ev;
	ev.data.fd = recs->control_pipe[ 0 ];
	errCode = epoll_ctl (recs->epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (errCode == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in EPOLL_CTL_DEL(1)", errno, strerror (errno));
		throw errControl (msgText);
	}
#endif
#endif
	
	/*
	 * remove all descriptors that wants auto_destroy
	 */
	for (size_t i = 0; i < descriptors_list.size (); ++i)
	{
		if (descriptors_list[ i ])
		{
#if defined (EPOLL_IO_MULTIPLEXING)
			/* here we do trust on the previously allocated ev object */
			ev.data.fd = descriptors_list[ i ]->get_descriptor ();
			errCode = epoll_ctl (recs->epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
			if (errCode == -1)
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in EPOLL_CTL_DEL(2)", errno, strerror (errno));
				throw errControl (msgText);
			}
#endif
#endif
			if (descriptors_list[ i ]->auto_destroy ())
				delete descriptors_list[ i ];
			descriptors_list[ i ] = 0;
		}
	}
	
#if defined (EPOLL_IO_MULTIPLEXING)
	/*
	 * close epoll file descriptor
	 */
	errCode = close (recs->epollfd);
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
	if (errCode == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in close", errno, strerror (errno));
		throw errClose (msgText);
	}
#endif
#endif
	
	/*
	 * release the rest of allocated data
	 */
	delete recs;
	recs = 0;
}

int event_loop::do_poll ( const int timeout )
{
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	for (int nfds = 0;;)
	{
#if defined (EPOLL_IO_MULTIPLEXING)
		nfds = epoll_wait (recs->epollfd, recs->events.data (), recs->events.size (), timeout);
#elif defined (POLL_IO_MULTIPLEXING)
		nfds = poll (recs->events.data (), recs->events.size (), timeout);
#elif defined (SELECT_IO_MULTIPLEXING)
		/* to be implemented in future (just for fun!) */
#endif
		if (nfds == -1)
		{
			if (errno != EINTR)
			{
				/* any error but not the signal interrupt */
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in event_loop::do_poll", errno, strerror (errno));
				throw errPoll (msgText);
			}
		}
		else
			return nfds;	/* a value of 0 indicates that the call timed	*/
						/* out and no file descriptors were ready		*/
	}
}

void event_loop::add_descriptor ( event_descriptor * d )
{
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	
	/*
	 * store event_descriptor object pointer
	 */
	descriptors_list.resize (d->get_descriptor () + 1, 0);
	descriptors_list[ d->get_descriptor () ] = d;
	
#if defined (EPOLL_IO_MULTIPLEXING)
	/*
	 * store event_descriptor object pointer if it needs special preparation
	 */
	if (d->needs_prepare ())
		recs->prepare_list.push_back (d);
	
	/*
	 * add file descriptor into the epoll descriptors list
	 */
	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = d->get_descriptor ();
	if (epoll_ctl (recs->epollfd, EPOLL_CTL_ADD, d->get_descriptor (), &ev) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in EPOLL_CTL_ADD", errno, strerror (errno));
		throw errControl (msgText);
	}
	
	/*
	 * track the number of descriptors
	 * actually we'll need this structure for epoll_wait
	 */
	recs->events.push_back (ev);
#endif
}

void event_loop::remove_descriptor ( event_descriptor * d )
{
	int number_of_descriptors_stored = descriptors_list.size ();
	if ((d->get_descriptor () < number_of_descriptors_stored) && !descriptors_list[ d->get_descriptor () ])
	{
		/* we don't care about this descriptor */
		return;
	}
	
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	
#if defined (EPOLL_IO_MULTIPLEXING)
	/*
	 * remove descriptor from the epoll's internal list
	 */
	epoll_event ev;
	ev.data.fd = d->get_descriptor ();
	if (epoll_ctl (recs->epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in EPOLL_CTL_DEL(2)", errno, strerror (errno));
		throw errControl (msgText);
	}
	
	/*
	 * remove descriptor from the prepare_list
	 */
	if (d->needs_prepare ())
	{
		for (size_t i = 0; i < recs->prepare_list.size (); ++i)
			if (recs->prepare_list[ i ] == d)
			{
				std::swap (recs->prepare_list[ i ], recs->prepare_list.back ());
				recs->prepare_list.pop_back ();
				break;
			}
	}
	
	/*
	 * track the number of descriptors
	 * only the number of descriptors is important so we just throw out the last one
	 * we needs only actually we'll need this structure for epoll_wait
	 */
	recs->events.pop_back ();
#endif
	
	/*
	 * and finally remove it from the descriptors list
	 */
	if (d->auto_destroy ())
		delete descriptors_list[ d->get_descriptor () ];
	descriptors_list[ d->get_descriptor () ] = 0;
}

void event_loop::prepare ()
{
	CrossClass::_LockIt descriptors_list_lock ( descriptors_list_mutex );
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	
#if defined (EPOLL_IO_MULTIPLEXING)
	epoll_event ev;
	for (size_t i = 0; i < recs->prepare_list.size (); )
	{
		ev.data.fd = recs->prepare_list[ i ]->get_descriptor ();
		if (descriptors_list[ ev.data.fd ] == 0)
		{
			/* remove file descriptor from the epoll descriptors list */
			if (epoll_ctl (recs->epollfd, EPOLL_CTL_DEL, ev.data.fd, &ev) == -1)
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in EPOLL_CTL_DEL", errno, strerror (errno));
				throw errControl (msgText);
			}
			
			/* keep track the number of descriptors */
			recs->events.pop_back ();
			
			/* destroy object if necessary */
			if (recs->prepare_list[ i ]->auto_destroy ())
				delete recs->prepare_list[ i ];
			
			/* remove it from the prepare list */
			std::swap (recs->prepare_list[ i ], recs->prepare_list.back ());
			recs->prepare_list.pop_back ();
		}
		else
		{
			/*
			 * change polling status if necessary
			 */
			if (recs->prepare_list[ i ]->want2write ())
				ev.events = EPOLLIN|EPOLLOUT;
			else
				ev.events = EPOLLIN;
			
			/*
			 * modify file descriptor in the epoll descriptors list
			 */
			if (epoll_ctl (recs->epollfd, EPOLL_CTL_MOD, ev.data.fd, &ev) == -1)
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in EPOLL_CTL_MOD", errno, strerror (errno));
				throw errControl (msgText);
			}
			
			++i;
		}
	}
#elif defined (POLL_IO_MULTIPLEXING)
	/*
	 * make sure we have enough space allocated
	 */
	recs->events.reserve (descriptors_list.size () + 1);
	
	/*
	 * control_pipe is the first descriptor for the poll
	 */
	recs->events.resize (1);
	recs->events[ 0 ].fd = recs->control_pipe[ 0 ];
	recs->events[ 0 ].events = POLLIN;
	
	/*
	 * fill in the rest pollfd stuctures for the rest of descriptors
	 */
	pollfd ev;
	for (size_t i = 0; i < descriptors_list.size (); )
	{
		if (descriptors_list[ i ] == 0)
		{
			std::swap (descriptors_list[ i ], descriptors_list.back ());
			descriptors_list.pop_back ();
		}
		else
		{
			ev.fd = descriptors_list[ i ]->get_descriptor ();
			ev.events = POLLIN;
			if (descriptors_list[ i ]->want2write ())
				ev.events |= POLLOUT;
			recs->events.push_back (ev);
			++i;
		}
	}
#elif defined (SELECT_IO_MULTIPLEXING)
#endif
}

bool event_loop::handle_events ( const int nfds )
{
	/*
	 * nothing to do if poll was timed out
	 */
	if (nfds == 0)
		return true;
	
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	
	/*
	 * exit imediately if poll was hardly forced to restart
	 */
	if (recs->restart_flag.load ())
	{
		recs->restart_flag.store (false);
		return true;
	}
	
	/*
	 * check and process ready descriptors
	 */
	CrossClass::_LockIt descriptors_list_lock ( descriptors_list_mutex );
#if defined (EPOLL_IO_MULTIPLEXING)
	for (int n = 0; n < nfds; ++n)
	{
		try
		{
			if (recs->events[ n ].events & (EPOLLERR|EPOLLHUP))
				throw int (-1);
			
			if (recs->events[ n ].events & EPOLLIN)
			{
				if (recs->events[ n ].data.fd == recs->control_pipe[ 0 ])
				{
					if (recs->check_terminate ())
					{
						notify_terminate ();
						return false;
					}
					else
						continue;
				}
				else
					descriptors_list[ recs->events[ n ].data.fd ]->handle_read ();
			}
			
			if (recs->events[ n ].events & EPOLLOUT)
				descriptors_list[ recs->events[ n ].data.fd ]->handle_write ();
		}
		catch (...)
		{
			descriptors_list[ recs->events[ n ].data.fd ]->handle_disconnect ();
			remove_descriptor (descriptors_list[ recs->events[ n ].data.fd ]);
		}
	}
#elif defined (POLL_IO_MULTIPLEXING)
	/*
	 * poll routine doesn't change the order of file descriptors
	 * therefore we could check control pipe imediately
	 * and of course we have to check every file descriptor in the list
	 */
	int nfds_last = nfds;
	if (recs->events[ 0 ].revents & POLLIN)
	{
		--nfds_last;
		if (recs->check_terminate ())
		{
			notify_terminate ();
			return false;
		}
	}
	
	for (int n = 1; (n < recs->events.size ()) && (0 < nfds_last); ++n)
	{
		if (recs->events[ n ].revents & (POLLERR|POLLHUP|POLLIN|POLLOUT|POLLNVAL))
			--nfds_last;
		
		try
		{
			if (recs->events[ n ].revents & (POLLERR|POLLHUP))
				throw int (-1);
			
			if (recs->events[ n ].revents & POLLIN)
				descriptors_list[ recs->events[ n ].fd ]->handle_read ();
			
			if (recs->events[ n ].events & POLLOUT)
				descriptors_list[ recs->events[ n ].fd ]->handle_write ();
		}
		catch (...)
		{
			descriptors_list[ recs->events[ n ].fd ]->handle_disconnect ();
			remove_descriptor (descriptors_list[ recs->events[ n ].fd ]);
		}
	}
#elif defined (SELECT_IO_MULTIPLEXING)
#endif
	return true;
}

void event_loop::restart ( const bool fHardRestart )
{
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	if (fHardRestart)
		recs->restart_flag.store (true);
	recs->pipe_out ('R');
}

void event_loop::terminate ( const bool fWaitConfirmation )
{
	poll_records * recs = reinterpret_cast<poll_records *> (records);
	recs->pipe_out ('T');
	if (fWaitConfirmation)
	{
		CrossClass::_LockIt terminate_lock ( terminate_mutex );
		while (!terminate_flag)
			terminate_condition.wait (terminate_lock);
		terminate_flag = false;
	}
}

void event_loop::add ( event_descriptor & d )
/* add an event descriptor to the poll */
{
	{
		CrossClass::_LockIt descriptors_list_lock ( descriptors_list_mutex );
		add_descriptor (&d);
	}
	restart (false);
}

void event_loop::remove ( event_descriptor & d )
/* remove an event descriptor from the poll */
{
	restart (true);
	{
		CrossClass::_LockIt descriptors_list_lock ( descriptors_list_mutex );
		remove_descriptor (&d);
	}
}

} /* namespace CrossClass	*/
