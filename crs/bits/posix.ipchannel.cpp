/*
 *  crs/bits/posix.ipchannel.cpp - event loop base interprocess communication
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
#include <crs/security.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>

namespace CrossClass {

/************************************************************/
/*										*/
/* members of class posix_ip_channel::read_pipe_descriptor	*/
/*										*/
/************************************************************/
posix_ip_channel::read_pipe_descriptor::~read_pipe_descriptor ()
{
	if (_local_callback_data)
		delete _in_message_data;
	
	int errCode = close (_read_pipe_end);
	if (errCode == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in read_pipe_descriptor::~read_pipe_descriptor", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
}

bool posix_ip_channel::read_pipe_descriptor::handle_read ()
/* return true if a message was received	*/
{
	if (_have_message)
	{
		_in_message_data->size = _next_byte_ptr - _in_message_data->message;
		if (_in_message_callback (_in_message_data))
		{
			_next_byte_ptr = _in_message_data->message;
			_have_message = false;
		}
		else
			return true;/* buffer is not free - we can not proceed	*/
	}
	
	ssize_t nBytesRead = 0;
	ssize_t nBytesFree = 0;
	
	for (;;)
	{
		nBytesFree = _in_message_data->allocated_size - (_next_byte_ptr - _in_message_data->message);
		nBytesRead = read (_read_pipe_end, _next_byte_ptr, nBytesFree);
		if (nBytesRead == -1)
		{
			if (errno == EINTR)
				continue;
			else if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
				break;		/* read buffer is empty				*/
			else
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in read_pipe_descriptor::handle_read", errno, strerror (errno));
				throw std::runtime_error (msgText);
			}
		}
		else if (nBytesRead == 0)
		{
			/* we need to disconnect ourself, because other end was closed	*/
			/* so, we need to throw something and event loop will disconnect	*/
			/* us after this									*/
			/*											*/
			/* but first of all let's notify about new message in buffer	*/
			if ((_next_byte_ptr != _in_message_data->message) && (_in_message_callback != 0))
			{
				_in_message_data->size = _next_byte_ptr - _in_message_data->message;
				_in_message_callback (_in_message_data);
			}
			throw int (0);
		}
		
		nBytesFree -= nBytesRead;
		_next_byte_ptr += nBytesRead;
		
		if (nBytesFree == 0)	/* our buffer is full					*/
		{
			if (_in_message_callback != 0)
			{
				_in_message_data->size = _next_byte_ptr - _in_message_data->message;
				if (_in_message_callback (_in_message_data))
					_next_byte_ptr = _in_message_data->message;
			}
			break;
		}
	}
	
	/* we are here, because receive buffer is empty or our buffer is full		*/
	if (_in_message_callback == 0)
	{
		_next_byte_ptr = _in_message_data->message;
		return false;
	}
	else
	{
		_in_message_data->size = _next_byte_ptr - _in_message_data->message;
		if (_in_message_callback (_in_message_data))
		{
			_next_byte_ptr = _in_message_data->message;
			return false;
		}
		else
		{
			_have_message = true;
			return true;
		}
	}
}

/************************************************************/
/*										*/
/* members of class posix_ip_channel::write_pipe_descriptor	*/
/*										*/
/************************************************************/
posix_ip_channel::write_pipe_descriptor::~write_pipe_descriptor ()
{
	delete [] _buffer;
	
	int errCode = close (_write_pipe_end);
	if (errCode == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in write_pipe_descriptor::~write_pipe_descriptor", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
}

bool posix_ip_channel::write_pipe_descriptor::handle_write ()
{
	_LockIt transmission_lock (_transmission_mutex);
	ssize_t nBytes2Write = _message_size - (_next_byte_ptr - _buffer);
	
	for (ssize_t nBytesWritten = 0; nBytes2Write > 0; )
	{
		nBytesWritten = write (_write_pipe_end, _next_byte_ptr, nBytes2Write);
		if (nBytesWritten == -1)
		{
			if (errno == EINTR)
				continue;
			else if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
				break;		/* write buffer is full				*/
			else
			{
				char msgText [ 256 ];
				sprintf (msgText, "%d: '%s' in write_pipe_descriptor::handle_write", errno, strerror (errno));
				throw std::runtime_error (msgText);
			}
		}
		
		_next_byte_ptr += nBytesWritten;
		nBytes2Write -= nBytesWritten;
	}
	
	if (nBytes2Write == 0)
	{
		/* message transmitted completely			*/
		_next_byte_ptr = _buffer;
		_message_size = 0;
		
		/* signal thread that transmission is done	*/
		_transmission_flag = false;
		_transmission_condition.notify_one ();
		return false;
	}
	else
		return true;
}

bool posix_ip_channel::write_pipe_descriptor::want2write ()
{
	_LockIt transmission_lock (_transmission_mutex);
	return _transmission_flag;
}

bool posix_ip_channel::write_pipe_descriptor::push ( const char * msg, const size_t msg_size, try_and_push )
{
	_LockIt transmission_lock (_transmission_mutex);
	if (!_transmission_flag)
	{
		_message_size = (msg_size < _allocated_buffer_size) ? msg_size : _allocated_buffer_size;
		memcpy (_buffer, msg, _message_size);
		_transmission_flag = true;
	}
	return _transmission_flag;
}

bool posix_ip_channel::write_pipe_descriptor::push ( const char * msg, const size_t msg_size, block_and_push )
{
	_LockIt transmission_lock (_transmission_mutex);
	while (_transmission_flag)
		_transmission_condition.wait (transmission_lock);
	_message_size = (msg_size < _allocated_buffer_size) ? msg_size : _allocated_buffer_size;
	memcpy (_buffer, msg, _message_size);
	_transmission_flag = true;
	return _transmission_flag;
}

/************************************************************/
/*										*/
/* members of class posix_ip_channel				*/
/*										*/
/************************************************************/
posix_ip_channel::posix_ip_channel (event_loop * ev_loop, incoming_message_callback cb, const size_t msgSize)
	: _pip ()
	, _eloop (ev_loop)
	, _input (0)
	, _output (0)
{
	if (_eloop == 0)
		throw std::runtime_error ("event_loop must be specified");
	
	/*
	 * create an inter process communication pipe
	 */
	if (pipe (_pip) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in posix_ip_channel::posix_ip_channel", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	/*
	 * modify flags for read pipe end
	 */
	int flags;
	if (fcntl (_pip[ 0 ], F_GETFL, &flags) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in posix_ip_channel::posix_ip_channel (fcntl-0-get)", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	flags |= O_NONBLOCK;
	if (fcntl (_pip[ 0 ], F_SETFL, &flags) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in posix_ip_channel::posix_ip_channel (fcntl-0-set)", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	/*
	 * modify flags for write pipe end
	 */
	if (fcntl (_pip[ 1 ], F_GETFL, &flags) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in posix_ip_channel::posix_ip_channel (fcntl-1-get)", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	flags |= O_NONBLOCK;
	if (fcntl (_pip[ 1 ], F_SETFL, &flags) == -1)
	{
		char msgText [ 256 ];
		sprintf (msgText, "%d: '%s' in posix_ip_channel::posix_ip_channel (fcntl-1-set)", errno, strerror (errno));
		throw std::runtime_error (msgText);
	}
	
	/*
	 * allocate event_descriptor objetcs
	 */
	_input = new read_pipe_descriptor (_pip[ 0 ], msgSize, cb);
	_output = new write_pipe_descriptor (_pip[ 1 ], msgSize);
	
	/*
	 * add them to the event loop
	 */
	_eloop->add (*_input);
	_eloop->add (*_output);
}

posix_ip_channel::~posix_ip_channel ()
{
	try
	{
		/*
		 * remove event_descriptor objects from the event loop
		 */
		_eloop->remove (*_output);
		_eloop->remove (*_input);
		
		/*
		 * and finaly - destroy them
		 */
		delete _output;
		delete _input;
	}
	catch (...)
	{
#if defined (DESTRUCTOR_EXCEPTIONS_ALLOWED)
		throw;
#endif
	}
}

}	/* namespace CrossClass		*/
