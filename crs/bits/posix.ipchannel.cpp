/*
 *  crs/bits/posix.ipchannel.cpp - event loop base inter process communication
 *                                 channel (pipe)
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

#include <crs/bits/posix.ipchannel.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

namespace CrossClass {

template <class ExceptionType, bool fromDestructor>
void throw_exception (const char * place, const int errcode, const char * errtext)
{
#if defined (DESTRUCTORS_EXCEPTIONS_ALLOWED)
	const bool can_throw = true;
#else
	const bool can_throw = !fromDestructor;
#endif
	if (can_throw)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in %s", errcode, errtext, place);
		throw ExceptionType (msgText);
	}
}

/************************************************************/
/*										*/
/* members of class posix_ip_channel::read_pipe_descriptor	*/
/*										*/
/************************************************************/
posix_ip_channel::read_pipe_descriptor::~read_pipe_descriptor ()
{
	if (close (_read_pipe_end) == -1)
		/* this exception should always be thrown		*/
		throw_exception<std::runtime_error, false> ("read_pipe_descriptor::~read_pipe_descriptor", errno, strerror (errno));
}

bool posix_ip_channel::read_pipe_descriptor::handle_read ()
/* return true if a message was received	*/
{
	for (ssize_t nBytesRead = 0;;)
	{
		nBytesRead = read (_read_pipe_end, _in_message_data->message, _in_message_data->allocated_size);
		if (nBytesRead == -1)
		{
			if (errno == EINTR)
				continue;
			else if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
				break;		/* read buffer is empty				*/
			else
				throw_exception<std::runtime_error, false> ("read_pipe_descriptor::handle_read", errno, strerror (errno));
		}
		else if (nBytesRead == 0)
		{
			/* we need to disconnect ourself, because other end was closed	*/
			/* event loop will disconnect us after this				*/
			throw int (-1);
		}
		else
		{
			_in_message_data->size = nBytesRead;
			_in_message_callback (_in_message_data);
			break;
		}
	}
	return false;
}

/************************************************************/
/*										*/
/* members of class posix_ip_channel::write_pipe_descriptor	*/
/*										*/
/************************************************************/
posix_ip_channel::write_pipe_descriptor::~write_pipe_descriptor ()
{
	if (close (_write_pipe_end) == -1)
		/* this exception should always be thrown		*/
		throw_exception<std::runtime_error, false> ("write_pipe_descriptor::~write_pipe_descriptor", errno, strerror (errno));
}

void posix_ip_channel::write_pipe_descriptor::push (const char * msg, const size_t msg_size)
{
	ssize_t bytes_rest = msg_size;
	for (ssize_t bytes_written = 0; bytes_rest > 0; )
	{
		bytes_written = write (_write_pipe_end, msg, bytes_rest);
		if ((bytes_written == -1) && (errno != EINTR))
			throw_exception<std::runtime_error, false> ("write_pipe_descriptor::handle_write", errno, strerror (errno));
		bytes_rest -= bytes_written;
		msg += bytes_written;
	}
}

/************************************************************/
/*										*/
/* members of class posix_ip_channel				*/
/*										*/
/************************************************************/
posix_ip_channel::posix_ip_channel (event_loop * ev_loop, incoming_message_callback cb, incoming_message_callback_data * cb_data)
	: basic_ip_channel (ev_loop)
	, _pip ()
{
	/*
	 * create an inter process communication pipe
	 */
	if (pipe (_pip) == -1)
		throw_exception<std::runtime_error, false> ("posix_ip_channel::posix_ip_channel", errno, strerror (errno));
	
	/*
	 * modify flags for read pipe end
	 */
	long flags = fcntl (_pip[ 0 ], F_GETFL);
	if (flags == -1)
		throw_exception<std::runtime_error, false> ("posix_ip_channel::posix_ip_channel (fcntl-0-get)", errno, strerror (errno));
	
	flags |= O_NONBLOCK;
	if (fcntl (_pip[ 0 ], F_SETFL, flags) == -1)
		throw_exception<std::runtime_error, false> ("posix_ip_channel::posix_ip_channel (fcntl-0-set)", errno, strerror (errno));
	
	/*
	 * allocate event_descriptor objetcs
	 */
	_input = new read_pipe_descriptor (_pip[ 0 ], cb, cb_data);
	_output = new write_pipe_descriptor (_pip[ 1 ]);
	
	/*
	 * add read pipe end to the event loop
	 */
	_eloop->add (*_input);
}

}	/* namespace CrossClass		*/
