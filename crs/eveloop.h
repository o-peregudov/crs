#ifndef CROSS_EVELOOP_H_INCLUDED
#define CROSS_EVELOOP_H_INCLUDED 1
/*
 *  crs/eveloop.h - event loop service class (a-la GMainLoop)
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
 *	2012/08/19	event loop service class (a-la GMainLoop)
 *	2012/08/29	remove method for several descriptors
 *	2012/09/21	template method for exception throw
 */

#include <crs/condition_variable.hpp>
#include <crs/callback.h>
#include <stdexcept>
#include <vector>
#include <list>

namespace CrossClass {

  class CROSS_EXPORT event_descriptor
  {
  public:
    virtual bool handle_read () = 0;		/* return true if a message was received	*/
    virtual bool handle_write () = 0;		/* return true if a send should be pending	*/
    
    virtual crs_fd_t get_descriptor () = 0;
	
    virtual bool needs_prepare () = 0;		/* needs preprocessing before polling		*/
    virtual bool want2write () = 0;		/* transmission is pending			*/
    virtual bool auto_destroy () = 0;		/* destroy object by 'event_loop' class		*/
	
    virtual void handle_disconnect ()
    {
      if (handle_disconnect_cb)
	handle_disconnect_cb (handle_disconnect_cb_data);
    }
	
    void set_disconnect_cb ( callBackFunction new_disconnect_cb, void * pData )
    {
      handle_disconnect_cb = new_disconnect_cb;
      handle_disconnect_cb_data = pData;
    }
	
    virtual ~event_descriptor () { }
	
  protected:
    event_descriptor ()
      : handle_disconnect_cb (0)
      , handle_disconnect_cb_data (0)
      { }
	
    callBackFunction handle_disconnect_cb;
    void * handle_disconnect_cb_data;
  };

  class CROSS_EXPORT event_loop
  {
  protected:
    std::vector<event_descriptor *>	descriptors_list;
    cMutex				descriptors_list_mutex,
					terminate_mutex;
    cConditionVariable			terminate_condition;
    bool				terminate_flag;
    void *				records;
	
    void notify_terminate ()
    /* termination feedback */
    {
      CrossClass::_LockIt terminate_lock ( terminate_mutex );
      terminate_flag = true;
      terminate_condition.notify_one ();
    }
    
    /*
     * lock free versions for cases when you need to call it from the 'handle_events'
     */
    void add_descriptor ( event_descriptor * d );
    void remove_descriptor ( event_descriptor * d );
    
  public:
    struct errCreate : std::runtime_error {
      errCreate ( const std::string & wh ) : std::runtime_error( wh ) { }
    };
    
    struct errClose : std::runtime_error {
      errClose ( const std::string & wh ) : std::runtime_error( wh ) { }
    };
    
    struct errControl : std::runtime_error {
      errControl ( const std::string & wh ) : std::runtime_error( wh ) { }
    };
    
    struct errPoll : std::runtime_error {
      errPoll ( const std::string & wh ) : std::runtime_error( wh ) { }
    };
    
    event_loop ( const size_t ExpectedNumberOfDescriptors );
    ~event_loop ();
    
  public:
    void add ( event_descriptor & d );
    /* add an event descriptor for the poll */
    
    void remove ( event_descriptor & d );
    /* remove an event descriptor from the poll */
    
    template <class InputIterator>
      void remove ( InputIterator from, InputIterator to )
      {
	restart (true);
	{
	  CrossClass::_LockIt descriptors_list_lock ( descriptors_list_mutex );
	  for (InputIterator p = from; p != to; ++p)
	    remove_descriptor (*p);
	}
      }
    
    void prepare ();
    /* check, whether we want to remove something or we need to modify events to poll for */
    
    int do_poll ( const int timeout = -1 /* wait indefinitely */);
    /* return the number of ready descriptors or 0 if timed out */
    
    bool handle_events ( const int nfds );
    /* should return false when termination event acquired */
    
    void restart ( const bool fHardRestart = false );
    /* restart event loop, if fHardRestart is true then ignores handle_event */
    
    void terminate ( const bool fWaitConfirmation = false );
    /* post termination event to the event loop and blocks if you need a confirmation of the termination */
    
    bool runOnce ( const int timeout = -1 /* wait indefinitely */)
    /* single iteration of the event loop */
    {
      prepare ();
      return handle_events (do_poll (timeout));
    }
  };

}	/* namespace CrossClass		*/
#endif	/* CROSS_EVELOOP_H_INCLUDED	*/
