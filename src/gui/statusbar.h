
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "gui_shared.h"


class StatusBar : public wxStatusBar
{
private:
	DECLARE_EVENT_TABLE()

public:
	StatusBar( wxWindow *parent );
	virtual ~StatusBar() {}

	void OnSize( wxSizeEvent& ev );

	void SetFPS( float fps );
	void SetName( const std::string& name );
};


#endif
