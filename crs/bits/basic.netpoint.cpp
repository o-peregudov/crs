/*
 *  crs/bits/basic.netpoint.cpp
 *  Copyright (c) 2009-2012 Oleg N. Peregudov <o.peregudov@gmail.com>
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

#include <crs/bits/basic.netpoint.h>
#include <algorithm>
#include <cstdio>
#include <cerrno>

#define EMSGLENGTH	256
#define MAXCLIENTS	256

namespace CrossClass {

//
// members of class netPoint
//
basicNetPoint::basicNetPoint ()
	: _socket (AF_INET, SOCK_STREAM, 0)
	, _sockAddress ()
	, _clientList ()
{
	_clientList.reserve (MAXCLIENTS);
	if (_socket == -1)
		throw socket_allocation_error ("basicNetPoint::basicNetPoint");
}

basicNetPoint::basicNetPoint ( cSocket & clientSocket )
	: _socket (clientSocket)
	, _sockAddress ()
	, _clientList ()
{
	_clientList.reserve (MAXCLIENTS);
}

basicNetPoint::~basicNetPoint ()
{
	disconnectAll ();
}

void basicNetPoint::disconnectAll ()
{
	for (size_t i = 0; i < _clientList.size (); removeClient (i++));
}

void basicNetPoint::addClient ( basicNetPoint * newClient )
{
	_clientList.push_back (newClient);
}

void basicNetPoint::removeClient ( const size_t nClient )
{
	if (handleDisconnect (_clientList[ nClient ]))
	{
		delete _clientList[ nClient ];
		_clientList[ nClient ] = 0;
	}
}

void basicNetPoint::clientReceive ( const size_t nClient )
{
	_clientList[ nClient ]->receive ();
}

void basicNetPoint::clientTransmit ( const size_t nClient )
{
	_clientList[ nClient ]->transmit ();
}

void basicNetPoint::setNonBlock ()
{
}

void basicNetPoint::bindSocket ( const cSockAddr & sa )
{
	setNonBlock();
	
	_sockAddress = sa;
	
	if (bind (_socket, sa, sizeof (sa)) == -1)
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "%d: %s", errno, strerror (errno));
		throw socket_bind_error (msgText);
	}
	
	if( listen( _socket, 16 ) == -1 )
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "%d: %s", errno, strerror (errno));
		throw socket_listen_error (msgText);
	}
}

void basicNetPoint::transmit ()
{
}

void basicNetPoint::receive ()
{
}

bool basicNetPoint::want2transmit ()
{
	return false;
}

basicNetPoint * basicNetPoint::handleNewConnection ( cSocket & s, const cSockAddr & sa )
{
	return 0;		/* new basicNetPoint (s);	*/
}

size_t basicNetPoint::enumerateDescriptors ()
{
	return _clientList.size ();
}

void basicNetPoint::buildSelectList ()
{
}

bool basicNetPoint::handleDisconnect ( basicNetPoint * )
{
	return true;
}

bool basicNetPoint::checkTerminate ()
{
	return true;	/* yes, we have to stop!	*/
}

void basicNetPoint::startServer ( const unsigned short int portNo )
{
	cSockAddr addrAny ( portNo );
	bindSocket (addrAny);
}

void basicNetPoint::clientConnect ( const cSockAddr & sa )
{
	if (connect (_socket, sa, sizeof (sa)) == -1)
	{
		char msgText [ EMSGLENGTH ];
		sprintf (msgText, "%d: %s", errno, strerror (errno));
		throw socket_connect_error (msgText);
	}
	else
		setNonBlock ();
}

void basicNetPoint::postRestart ()
{
}

void basicNetPoint::postTerminate ()
{
}

bool basicNetPoint::clientSendRecv ()
{
	return true;
}

bool basicNetPoint::serverSendRecv ()
{
	return true;
}

cSocket & basicNetPoint::getSocket ()
{
	return _socket;
}

} /* namespace CrossClass	*/
