//
// Timer.cpp: interface for the cTimer class.
// (c) May 22, 2007 Oleg N. Peregudov
// Oct 17, 2007 - heavy usage of C++ library
// Nov 21, 2007 - new place for cross-compiling routines
// Feb 11, 2008
// Jan 3, 2009
// Apr 23, 2009 - Win/Posix defines
// Oct 21, 2009 - C++0x timers
// Jun 25, 2010 - C++0x mutexes
// Aug 21, 2010 - separate versions for each API
//

#include <crs/timer.h>
#include <sstream>
#include <iomanip>

namespace CrossClass {

std::string cTimer::time2string ( const double & tm, const bool fMSecs )
{
      double msTime = tm;
	std::basic_ostringstream<char> stream;
      int nHours = static_cast<int>( msTime / 3.6e6 );
      msTime -= nHours * 3.6e6;
      int nMinutes = static_cast<int>( msTime / 6e4 );
      msTime -= nMinutes * 6e4;
      int nSeconds = static_cast<int>( msTime / 1e3 );
	stream.fill( '0' );
	stream << nHours << ':' << std::setw( 2 ) << nMinutes << ':' << std::setw( 2 ) << nSeconds;
      if( fMSecs )
		stream << '.' << std::setw( 3 ) << static_cast<int>( msTime -= nSeconds * 1e3 );
	return stream.str();
}

} // namespace CrossClass

