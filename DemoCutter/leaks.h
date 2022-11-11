#ifndef LEAKS_H
#define LEAKS_H

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
   #ifndef DBG_NEW
      #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
      #define new DBG_NEW
   #endif
#endif  // _DEBUG


void setReportCout(){
	// Send all reports to STDOUT
   _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
}


#endif /* LEAKS_H */