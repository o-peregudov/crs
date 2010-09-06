// cross.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if( dwReason == DLL_PROCESS_ATTACH )
	{
	}
	else if( dwReason == DLL_PROCESS_DETACH )
	{
	}
	
	return 1;   // ok
}
