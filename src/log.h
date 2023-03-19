
#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <boost/format.hpp>
#include "wx/log.h"


enum LOG_TYPE
{
	LOG_CPU = 1,
	LOG_PPU = 2,
	LOG_MAPPER = 4,
	LOG_MISC = 8,
	LOG_ERROR = 16
};


class Log
{
private:
	static bool enable;
	static int typesEnabled;
	static std::ofstream mFile;

public:

	static void ToggleEnabled( int type );

	static void Enable( bool en );

	static void EnableType( int type );

	static void DisableType( int type );

	static bool IsTypeEnabled( int type );

	static void Open( wxWindow* parent );
	static void Close();

	static void Write( LOG_TYPE type, const std::wstring& message );
};


#define PWLOG0( type, text )																																	if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, text ); }
#define PWLOG1( type, text, arg1 )																														if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) ).str() ); }
#define PWLOG2( type, text, arg1, arg2 )																											if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) ).str() ); }
#define PWLOG3( type, text, arg1, arg2, arg3 )																								if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) ).str() ); }
#define PWLOG4( type, text, arg1, arg2, arg3, arg4 )																					if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) ).str() ); }
#define PWLOG5( type, text, arg1, arg2, arg3, arg4, arg5 )																		if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) ).str() ); }
#define PWLOG6( type, text, arg1, arg2, arg3, arg4, arg5, arg6 )															if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) % ( arg6 ) ).str() ); }
#define PWLOG7( type, text, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )												if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) % ( arg6 ) % ( arg7 ) ).str() ); }
#define PWLOG8( type, text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 )									if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) % ( arg6 ) % ( arg7 ) % ( arg8 ) ).str() ); }
#define PWLOG9( type, text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 )						if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) % ( arg6 ) % ( arg7 ) % ( arg8 ) % ( arg9 ) ).str() ); }
#define PWLOG10( type, text, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10 )		if ( Log::IsTypeEnabled( type ) ) { Log::Write( type, ( boost::wformat( text ) % ( arg1 ) % ( arg2 ) % ( arg3 ) % ( arg4 ) % ( arg5 ) % ( arg6 ) % ( arg7 ) % ( arg8 ) % ( arg9 ) % ( arg10 ) ).str() ); }




#endif
