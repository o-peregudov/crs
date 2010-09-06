#include "cross/ude/devman.h"
#include <vector>
#include <iostream>
#include <string>

#define NTHREADS	16

class cSampleDeviceManager : public ude::device_manager
{
protected:
	size_t nSendPackets;
	
      virtual void _send ( const ude::cTalkPacket & p )
	{
		//std::cout << reinterpret_cast<const char *>( p.byte() ) << std::flush;
		if( ++nSendPackets > 1000000 )
			evntDone.SetEvent();
	}
	
      virtual void _receive ( ude::cTalkPacket & )
	{
	}

public:
	CrossClass::cEvent evntDone;
	
	cSampleDeviceManager ()
		: ude::device_manager()
		, nSendPackets( 1 )
		, evntDone( false, true )
	{ }
	
	~cSampleDeviceManager ()
	{
	}
};

class cSampleDevice : public CrossClass::cThread, public ude::basic_device
{
protected:
	const std::string myName;
	size_t nHits;
	
	virtual bool Step ()
	{
		ude::cTalkPacket packet ( 0, myName.size() + 1 );
		myName.copy( reinterpret_cast<char *>( packet.byte() ), myName.size() );
		if( !_device_manager->register_request( this, packet ) )
			std::cout << myName << std::flush;
		++nHits;
		CrossClass::sleep( 0 );
		return false;
	}
      
public:
	cSampleDevice ( const std::string & name, ude::basic_device_manager * dm )
		: CrossClass::cThread( false )
		, ude::basic_device( dm )
		, myName( name )
		, nHits( 0 )
	{ }
	
	virtual ~cSampleDevice ()
	{
		std::cout << "'" << myName << "' hits " << nHits << " times" << std::endl;
	}
	
	virtual int take ( const ude::cTalkPacket & )
	{
		return 2;
	}
      
      virtual void reset ()
	{
	}
	
      virtual bool invariant ()
	{
		return true;
	}
};

int main ()
{
	{
		cSampleDeviceManager devMan;
		std::string alphabet ( "abcdefghijklmnopqrstu" );
		std::auto_ptr<cSampleDevice> thread [ NTHREADS ];
		for( size_t i = 0; i < NTHREADS; ++i )
		{
			thread[ i ] = std::auto_ptr<cSampleDevice>( new cSampleDevice( alphabet.substr( i, 1 ), &devMan ) );
			thread[ i ]->Resume();
		}
		
		devMan.evntDone.WaitEvent();
		std::cout << std::endl;
		
		for( size_t i = NTHREADS; i; thread[ --i ]->kill() );
	}
	std::cout << "well done!" << std::endl;
	return 0;
}
