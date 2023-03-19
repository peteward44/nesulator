

#include "main.h"
#include "log.h"


std::ofstream Log::mFile;


void Log::ToggleEnabled( int type )
{
	if ( !IsTypeEnabled( type ) )
		EnableType( type );
	else
		DisableType( type );
}


void Log::Enable( bool en )
{
	enable = en;
}


void Log::EnableType( int type )
{
	typesEnabled |= type;
}


void Log::DisableType( int type )
{
	typesEnabled &= ~type;
}


bool Log::IsTypeEnabled( int type )
{
	return ( typesEnabled & type ) == type;
}


void Log::Open( wxWindow* parent )
{
#ifdef ENABLE_LOGGING
// configure wx logging
::wxLog::SetActiveTarget( new wxLogWindow( parent, "Nesulator Debug Output" ) );
//::wxLog::SetLogLevel( 
::wxLog::EnableLogging();

	mFile.open( "log.txt" );
#endif
}


void Log::Close()
{
#ifdef ENABLE_LOGGING
	mFile.close();
#endif
}


void Log::Write( LOG_TYPE type, const std::wstring& message )
{
#ifdef ENABLE_LOGGING
	if ( enable && IsTypeEnabled( type ) )
	{
//		::wxLogMessage( message.c_str() );
//		::OutputDebugStringA( ( message + "\r\n" ).c_str() );
		mFile << message << std::endl;

		static int flushInterval = 0;
		flushInterval++;
		if ( flushInterval >= 10000 ) {
			mFile.flush();
			flushInterval = 0;
		}
	}
#endif
}

