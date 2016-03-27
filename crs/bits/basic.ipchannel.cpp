/*
 *  crs/bits/basic.ipchannel.cpp - event loop based inter process communication
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

#include <crs/bits/basic.ipchannel.h>
#include <cstring>

namespace CrossClass {

basic_ip_channel::basic_read_pipe_descriptor::basic_read_pipe_descriptor (incoming_message_callback cb, incoming_message_callback_data * cb_data)
	: _have_message (false)
	, _in_message_callback (cb)
	, _in_message_data (cb_data)
	, _next_byte_ptr (0)
	, _local_callback_data (false)
{
	if (_in_message_callback == 0)
		throw std::runtime_error ("incoming_message_callback must be specified");
	
	if (_in_message_data == 0)
	{
		_in_message_data = new incoming_message_callback_data (1024);
		_local_callback_data = true;
	}
	_next_byte_ptr = _in_message_data->message;
}

basic_ip_channel::basic_read_pipe_descriptor::~basic_read_pipe_descriptor ()
{
	if (_local_callback_data)
		delete _in_message_data;
}

basic_ip_channel::basic_ip_channel (event_loop * eloop)
	: _eloop (eloop)
	, _input (0)
	, _output (0)
{
	if (_eloop == 0)
		throw std::runtime_error ("event_loop must be specified");
}

basic_ip_channel::~basic_ip_channel ()
{
	try
	{
		/*
		 * remove event_descriptor objects from the event loop
		 */
		if (_input)
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
