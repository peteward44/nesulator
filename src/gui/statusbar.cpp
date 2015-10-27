

#include "statusbar.h"
#include "boost/format.hpp"


BEGIN_EVENT_TABLE(StatusBar, wxStatusBar)
    EVT_SIZE(StatusBar::OnSize)
END_EVENT_TABLE()


StatusBar::StatusBar( wxWindow *parent )
: wxStatusBar( parent )
{
	SetFieldsCount( 2 );
	SetStatusText( "Ready", 0 );
}


void StatusBar::OnSize( wxSizeEvent& ev )
{
}


void StatusBar::SetFPS( float fps )
{
	SetStatusText( ( boost::format( "FPS: %1$.2f" ) % fps ).str().c_str(), 1 );
}


void StatusBar::SetName( const std::string& name )
{
	SetStatusText( name.c_str(), 0 );
}
