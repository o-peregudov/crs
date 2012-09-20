#ifndef CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED
#define CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED 1
/*
 *  crs/bits/posix.ipchannel.h - event loop base interprocess communication
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

#include <crs/eveloop.h>

namespace CrossClass {

class CROSS_EXPORT posix_ip_channel
{
public:
	class incoming_message_callback_data
	{
		incoming_message_callback_data (incoming_message_callback_data &);
		incoming_message_callback_data & operator = (incoming_message_callback_data&);
		
	public:
		char			* message;
		void			* data;
		const size_t	allocated_size;
		size_t		size;
		
		incoming_message_callback_data (const size_t sz = 1024)
			: message (new char [ sz ])
			, data (0)
			, allocated_size (sz)
			, size (0)
		{ }
		
		~incoming_message_callback_data ()
		{
			delete [] message;
		}
	};
	
	typedef bool (*incoming_message_callback) (incoming_message_callback_data *);
	
	struct try_and_push {};
	struct block_and_push {};
	
protected:
	class read_pipe_descriptor : public event_descriptor
	{
		read_pipe_descriptor ( const read_pipe_descriptor & );
		read_pipe_descriptor & operator = ( const read_pipe_descriptor & );
		
	protected:
		crs_fd_t					_read_pipe_end;
		bool						_have_message;
		incoming_message_callback		_in_message_callback;
		incoming_message_callback_data	* _in_message_data;
		char						* _next_byte_ptr;
		bool						_local_callback_data;
		
	public:
		read_pipe_descriptor ( crs_fd_t fd, const size_t maxMsgSize, incoming_message_callback cb, incoming_message_callback_data * cb_data = 0 )
			: _read_pipe_end (fd)
			, _have_message (false)
			, _in_message_callback (cb)
			, _in_message_data (cb_data)
			, _next_byte_ptr (0)
			, _local_callback_data (false)
		{
			if (_in_message_data == 0)
			{
				_in_message_data = new incoming_message_callback_data (maxMsgSize);
				_local_callback_data = true;
			}
			_next_byte_ptr = _in_message_data->message;
		}
		
		virtual ~read_pipe_descriptor ();
		
		virtual bool handle_read ();	/* return true if a message was received		*/
		
	public:
		virtual bool handle_write ()
		{
			return false;		/* no pending write request				*/
		}
		
		virtual crs_fd_t get_descriptor ()
		{
			return _read_pipe_end;
		}
		
		virtual bool needs_prepare ()
		{
			return false;		/* do not need preprocessing before polling	*/
		}
		
		virtual bool want2write ()
		{
			return false;		/* no pending transmissions				*/
		}
		
		virtual bool auto_destroy ()
		{
			return false;		/* do not destroy object by 'event_loop' class	*/
		}
	};
	
	class write_pipe_descriptor : public event_descriptor
	{
		write_pipe_descriptor ( const write_pipe_descriptor & );
		write_pipe_descriptor & operator = ( const write_pipe_descriptor & );
		
	protected:
		crs_fd_t			_write_pipe_end;
		const size_t		_allocated_buffer_size;
		char				* _buffer;
		const char			* _next_byte_ptr;
		size_t			_message_size;
		cMutex			_transmission_mutex;
		bool				_transmission_flag;
		cConditionVariable	_transmission_condition;
		
	public:
		write_pipe_descriptor ( crs_fd_t fd, const size_t maxMsgSize )
			: _write_pipe_end (fd)
			, _allocated_buffer_size (maxMsgSize)
			, _buffer (new char [ maxMsgSize ])
			, _next_byte_ptr (_buffer)
			, _message_size (0)
			, _transmission_mutex ()
			, _transmission_flag (false)
			, _transmission_condition ()
		{
		}
		
		virtual ~write_pipe_descriptor ();
		
		virtual bool handle_write ();
		virtual bool want2write ();
		
		bool	push ( const char * msg, const size_t msg_size, try_and_push );
		bool	push ( const char * msg, const size_t msg_size, block_and_push );
		
	public:
		virtual bool handle_read ()
		{
			return false;		/* return true if a message was received		*/
		}
		
		virtual crs_fd_t get_descriptor ()
		{
			return _write_pipe_end;
		}
		
		virtual bool needs_prepare ()
		{
			return true;		/* we need preprocessing before polling		*/
		}
		
		virtual bool auto_destroy ()
		{
			return false;		/* do not destroy object by 'event_loop' class	*/
		}
	};
	
	crs_fd_t			_pip [ 2 ];
	event_loop			* _eloop;
	read_pipe_descriptor	* _input;
	write_pipe_descriptor	* _output;
	
public:
	posix_ip_channel (event_loop * ev_loop, incoming_message_callback cb, const size_t msgSize = 1024);
	~posix_ip_channel ();
	
	template <class push_type>
	bool	push ( const char * msg, const size_t msg_size, push_type )
	{
		if (_output->push (msg, msg_size, push_type ()))
		{
			_eloop->restart ();
			return true;
		}
		return false;
	}
};

}	/* namespace CrossClass				*/
#endif/* CROSS_BITS_POSIXIPCHANNEL_H_INCLUDED	*/
