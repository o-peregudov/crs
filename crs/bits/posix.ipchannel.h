#ifndef CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED
#define CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED 1
/*
 *  crs/bits/posix.ipchannel.h - event loop base inter process communication
 *                               channel (pipe)
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

/*
 *	2012/09/22	first version
 */

#include <crs/bits/basic.ipchannel.h>

namespace CrossClass {

class CROSS_EXPORT posix_ip_channel : public basic_ip_channel
{
protected:
	class read_pipe_descriptor : public basic_ip_channel::basic_read_pipe_descriptor
	{
	protected:
		crs_fd_t _read_pipe_end;
		
	public:
		read_pipe_descriptor ( crs_fd_t fd, incoming_message_callback cb, incoming_message_callback_data * cb_data )
			: basic_ip_channel::basic_read_pipe_descriptor (cb, cb_data)
			, _read_pipe_end (fd)
		{ }
		
		virtual ~read_pipe_descriptor ();
		
		virtual bool handle_read ();	/* return true if a message was received	*/
		
		virtual crs_fd_t get_descriptor ()
		{
			return _read_pipe_end;
		}
	};
	
	class write_pipe_descriptor : public basic_ip_channel::basic_write_pipe_descriptor
	{
	protected:
		crs_fd_t _write_pipe_end;
		
	public:
		write_pipe_descriptor ( crs_fd_t fd )
			: basic_ip_channel::basic_write_pipe_descriptor ()
			, _write_pipe_end (fd)
		{ }
		
		virtual ~write_pipe_descriptor ();
		
		virtual void push ( const char * msg, const size_t msg_size );
		
		virtual crs_fd_t get_descriptor ()
		{
			return _write_pipe_end;
		}
	};
	
	crs_fd_t _pip [ 2 ];
	
public:
	posix_ip_channel (event_loop * ev_loop, incoming_message_callback cb, incoming_message_callback_data * cb_data = 0);
	virtual ~posix_ip_channel () { }
};

}	/* namespace CrossClass				*/
#endif/* CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED	*/
