#ifndef CRS_MUTEX_H_INCLUDED
#define CRS_MUTEX_H_INCLUDED 1
/*
 *  crs/mutex.h
 *  Copyright (c) 2008-2016 Oleg N. Peregudov <o.peregudov@gmail.com>
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

#include <crs/libexport.h>

#if USE_CXX11_MUTEX
#  include <mutex>
namespace crs
{
  typedef std::mutex                   mutex_type;
  typedef std::unique_lock<mutex_type> lock_type;
}
#else
#  error This file requires ISO C++ 2011 standard <mutex> header, yet it was not detected on your system.
#endif
#endif /* CRS_MUTEX_H_INCLUDED */
