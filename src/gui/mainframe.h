
#ifndef GUI_MAINFRAME_H
#define GUI_MAINFRAME_H

#include "gui_shared.h"
#include "glcanvas.h"


class StatusBar : public wxStatusBar
{
private:
	DECLARE_EVENT_TABLE()

public:
	StatusBar( wxWindow *parent );
	virtual ~StatusBar() {}

	void OnSize( wxSizeEvent& ev );

	void SetFPS( float fps );
	void SetName( const std::wstring& name );
};


// Main window class for the application

const int AcceleratorEntries = 8;

class MainFrame : public wxFrame
{
private:
	GLCanvas* canvas;
	wxMenuBar* menuBar;
	StatusBar* statusBar;
	wxAcceleratorTable accelerators;
	wxAcceleratorEntry accelEntries[ AcceleratorEntries ];

	wxMenuItem *menuPal, *menuNtsc;
	wxMenuItem *loggingMenus[ 4 ];
	wxMenuItem *speedMenus[ 7 ];
	wxMenu* loadStateMenu, *saveStateMenu;

	DECLARE_EVENT_TABLE();

	std::wstring GetSaveStateFilename( int slot ) const;

public:

	MainFrame( int windowWidth, int windowHeight );
	virtual ~MainFrame();


	void OnQuit( wxCommandEvent& ev );

	void Run();

	void OnMenuLoad( wxCommandEvent& ev );
	void OnMenuLoadState( wxCommandEvent& ev );
	void OnMenuSaveState( wxCommandEvent& ev );
	void OnMenuDecompile( wxCommandEvent& ev );
	void OnMenuExit( wxCommandEvent& ev );
	void OnMenuAbout( wxCommandEvent& ev );
	void OnMenuOptions( wxCommandEvent& ev );

	void OnMenuNTSCMode( wxCommandEvent& ev );
	void OnMenuPALMode( wxCommandEvent& ev );

	void OnMenuLogging( wxCommandEvent& ev );

	void OnMenuColdReset( wxCommandEvent& ev );
	void OnMenuWarmReset( wxCommandEvent& ev );

	void OnMouseLeftDown( wxMouseEvent& ev );
	void OnMouseLeftUp( wxMouseEvent& ev );

	void OnMenuCartInfo( wxCommandEvent& ev );

	void OnMenuSize( wxCommandEvent& ev );
	void OnMenuSpeed( wxCommandEvent& ev );

	void OnSize( wxSizeEvent& ev );
	void OnKeyDown(wxKeyEvent& ev);
	void OnKeyUp(wxKeyEvent& ev);

	void OnClose(wxCloseEvent& event);

	inline GLCanvas* GetCanvas() { return canvas; }
	inline void SetFps( float fps ) { statusBar->SetFPS( fps ); }
	inline void SetRomName( const std::wstring& name ) { statusBar->SetName( name ); }
};



#endif

