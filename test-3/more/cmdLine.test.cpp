#include "cross/cmdLine.h"

void	usage ()
{
}

void	error ( const char * s )
{
	std::cerr << s;
}

//
// local command compiler
//
int   proceed( CommandLine::reader & command_line )
{
      try
      {
		std::string option;
            bool switchState ( true );
            
		std::cout << "Start ..." << std::endl;
		
            // get files and(or) options list
            for( CommandLine::reader::tDListIter i = command_line.begin(); i != command_line.end(); ++i )
            {
                  option = *i;
			if( CommandLine::reader::parseSwitch( option, switchState ) )
			{
				std::cout << "Switch '" << option << "', ";
				if( switchState )
					std::cout << "enabled";
				else
					std::cout << "disabled";
				std::cout << std::endl;
			}
			else
				std::cout << "Option '" << option << "'" << std::endl;
		}
		
            // execute command
            return 1;
      }
      catch ( std::runtime_error & e )
      {
            std::cerr  << "Runtime error in '" << e.what() << "'!" << std::endl;
      }
      return 0;
}

#include "cross/main"
