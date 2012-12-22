#ifndef CROSS_BITS_BASIC_IPCHANNEL_H_INCLUDED
#define CROSS_BITS_BASIC_IPCHANNEL_H_INCLUDED 1
/*
 *  crs/bits/basic.ipchannel.h - event loop based inter process communication
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
 *	2012/09/23	first version
 */

#include <crs/libexport.h>
#include <crs/eveloop.h>

namespace CrossClass {

class CROSS_EXPORT basic_ip_channel
{
	basic_ip_channel (const basic_ip_channel &);
	basic_ip_channel & operator = (const basic_ip_channel &);
	
public:
	class incoming_message_callback_data
	{
		incoming_message_callback_data (const incoming_message_callback_data &);
		incoming_message_callback_data & operator = (const incoming_message_callback_data&);
		
	public:
		char			* message;
		void			* data;
		const size_t	allocated_size;
		size_t		size;
		
		incoming_message_callback_data (const size_t sz = 1024, void * dptr = 0)
			: message (new char [ sz ])
			, data (dptr)
			, allocated_size (sz)
			, size (0)
		{ }
		
		~incoming_message_callback_data ()
		{
			delete [] message;
		}
	};
	
	typedef void (*incoming_message_callback) (incoming_message_callback_data *);
	
protected:
	class basic_read_pipe_descriptor : public event_descriptor
	{
		basic_read_pipe_descriptor ( const basic_read_pipe_descriptor & );
		basic_read_pipe_descriptor & operator = ( const basic_read_pipe_descriptor & );
		
	protected:
		bool						_have_message;
		incoming_message_callback		_in_message_callback;
		incoming_message_callback_data	* _in_message_data;
		char						* _next_byte_ptr;
		bool						_local_callback_data;
		
	public:
		basic_read_pipe_descriptor (incoming_message_callback cb, incoming_message_callback_data * cb_data);
		virtual ~basic_read_pipe_descriptor ();
		
		virtual bool handle_write ()	{ return false; }	/* nothing to transmit					*/
		virtual bool needs_prepare ()	{ return false; }	/* do not need preprocessing before polling	*/
		virtual bool want2write ()	{ return false; }	/* no pending transmissions				*/
		virtual bool auto_destroy ()	{ return false; }	/* do not destroy object by 'event_loop' class	*/
	};
	
	class basic_write_pipe_descriptor : public event_descriptor
	{
		basic_write_pipe_descriptor ( const basic_write_pipe_descriptor & );
		basic_write_pipe_descriptor & operator = ( const basic_write_pipe_descriptor & );
		
	public:
		basic_write_pipe_descriptor () { }
		virtual ~basic_write_pipe_descriptor () { }
		
		virtual void push ( const char * msg, const size_t msg_size ) = 0;
		
		virtual bool want2write ()	{ return false; } /* we'll use blocking mode				*/
		virtual bool handle_read ()	{ return false; }	/* return true if a message was received		*/
		virtual bool handle_write ()	{ return false; }	/* let's suppose that write was done		*/
		virtual bool needs_prepare ()	{ return false; }	/* we need preprocessing before polling		*/
		virtual bool auto_destroy ()	{ return false; }	/* do not destroy object by 'event_loop' class	*/
	};
	
protected:
	event_loop				* _eloop;
	basic_read_pipe_descriptor	* _input;
	basic_write_pipe_descriptor	* _output;
	
	basic_ip_channel (event_loop * eloop);
	
public:
	virtual ~basic_ip_channel ();
	
	void push ( const char * msg, const size_t msg_size )
	{
		_output->push (msg, msg_size);
	}
};

}	/* namespace CrossClass				*/
#endif/* CROSS_BITS_BASIC_IPCHANNEL_H_INCLUDED	*/
