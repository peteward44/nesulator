

#ifndef GUI_SHARED_H
#define GUI_SHARED_H

#include "wx/wx.h"


#ifdef WIN32

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "wsock32.lib")

//#ifdef _DEBUG
//
//#pragma comment(lib, "wxzlibd.lib")
//#pragma comment(lib, "wxregexd.lib")
//
//#pragma comment(lib, "wxbase28d.lib")
//#pragma comment(lib, "wxmsw28d_gl.lib")
//#pragma comment(lib, "wxmsw28d_core.lib")
//
//#else

#pragma comment(lib, "wxzlib.lib")
#pragma comment(lib, "wxregex.lib")

#pragma comment(lib, "wxbase28.lib")
#pragma comment(lib, "wxmsw28_gl.lib")
#pragma comment(lib, "wxmsw28_core.lib")

//#endif

#endif


#endif

