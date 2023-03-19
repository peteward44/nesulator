


#include "../main.h"
#include "mainframe.h"
#include "../mainboard.h"
#include "../decompiler.h"
#include "optionsdialog.h"
#include "app.h"

#include "wx/dnd.h"
#include "wx/filename.h"
#include "wx/statbox.h"

#include <algorithm>

using namespace boost::placeholders;

#define ENABLE_DECOMPILE


enum MainFrameEvents
{
	ID_MENULOAD,
	ID_MENUSAVESTATE,
	ID_MENUSAVESTATE_0,
	ID_MENUSAVESTATE_1,
	ID_MENUSAVESTATE_2,
	ID_MENUSAVESTATE_3,
	ID_MENULOADSTATE,
	ID_MENULOADSTATE_0,
	ID_MENULOADSTATE_1,
	ID_MENULOADSTATE_2,
	ID_MENULOADSTATE_3,
	ID_MENUDECOMPILE,
	ID_MENUEXIT,

	ID_MENUOPTIONS,
	ID_MENUCOLDRESET,
	ID_MENUWARMRESET,
	ID_MENUNTSCMODE,
	ID_MENUPALMODE,
	ID_MENUCARTINFO,

	ID_MENULOGALL,
	ID_MENULOGCPU,
	ID_MENULOGPPU,
	ID_MENULOGMAPPER,
	ID_MENULOGMISC,

	ID_MENUSIZE_0,
	ID_MENUSIZE_1,
	ID_MENUSIZE_2,
	ID_MENUSIZE_3,
	ID_MENUSIZE_FULLSCREEN,

	ID_MENUSPEED_0,
	ID_MENUSPEED_1,
	ID_MENUSPEED_2,
	ID_MENUSPEED_3,
	ID_MENUSPEED_4,
	ID_MENUSPEED_5,
	ID_MENUSPEED_6,

	ID_MENUABOUT
};


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(ID_MENUEXIT, MainFrame::OnMenuExit)
	EVT_MENU(ID_MENULOAD, MainFrame::OnMenuLoad)
	
	EVT_MENU_RANGE( ID_MENUSAVESTATE_0, ID_MENUSAVESTATE_3, MainFrame::OnMenuSaveState )
	EVT_MENU_RANGE( ID_MENULOADSTATE_0, ID_MENULOADSTATE_3, MainFrame::OnMenuLoadState )

	EVT_MENU_RANGE( ID_MENUSPEED_0, ID_MENUSPEED_6, MainFrame::OnMenuSpeed )

	EVT_MENU_RANGE( ID_MENULOGALL, ID_MENULOGMISC, MainFrame::OnMenuLogging )

	EVT_MENU(ID_MENUDECOMPILE, MainFrame::OnMenuDecompile)
	EVT_MENU(ID_MENUOPTIONS, MainFrame::OnMenuOptions)

	EVT_MENU(ID_MENUNTSCMODE, MainFrame::OnMenuNTSCMode)
	EVT_MENU(ID_MENUPALMODE, MainFrame::OnMenuPALMode)

	EVT_MENU(ID_MENUCARTINFO, MainFrame::OnMenuCartInfo)

	EVT_MENU(ID_MENUCOLDRESET, MainFrame::OnMenuColdReset)
	EVT_MENU(ID_MENUWARMRESET, MainFrame::OnMenuWarmReset)

	EVT_MENU(ID_MENUABOUT, MainFrame::OnMenuAbout)

	EVT_MENU_RANGE( ID_MENUSIZE_0, ID_MENUSIZE_FULLSCREEN, MainFrame::OnMenuSize )

	EVT_SIZE(MainFrame::OnSize)

	EVT_KEY_DOWN(MainFrame::OnKeyDown)
	EVT_KEY_UP(MainFrame::OnKeyUp)

	EVT_LEFT_DOWN( MainFrame::OnMouseLeftDown )
	EVT_LEFT_UP( MainFrame::OnMouseLeftUp )

	EVT_CLOSE(MainFrame::OnClose)

END_EVENT_TABLE()

/////////////////////////////////////////////


/////////////////////////////////////////////
// allows drag & drop of rom files

class FileDropTarget : public wxFileDropTarget
{
private:
	MainFrame* mainFrame;

public:
	FileDropTarget( MainFrame* mainFrame )
		: mainFrame( mainFrame )
	{}

	virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString& filenames )
	{
		try
		{
			wxGetApp().LoadRomFile( filenames[0].ToStdWstring() );
			return true;
		}
		catch ( std::exception& e )
		{
			NesulatorApp::DisplayError( e.what() );
			return false;
		}
	}
};


/////////////////////////////////////////////
// Cartridge information dialog

class CartridgeInfo : public wxDialog
{
private:

public:
	CartridgeInfo( wxWindow* parent, CartridgePtr_t cartridge )
		: wxDialog( parent, -1, "Cartridge Information", wxDefaultPosition, wxSize( 400, 200 ) )
	{
		//new wxStaticText( this, -1, "Name:", wxPoint( 10, 10 ) );
		//new wxStaticText( this, -1, cartridge->GetName().c_str(), wxPoint( 150, 10 ) );
		//new wxStaticText( this, -1, "CRC32:", wxPoint( 10, 40 ) );
		//new wxStaticText( this, -1, ( boost::format( "%1$X" ) % cartridge->GetCRC32() ).str().c_str(), wxPoint( 150, 40 ) );
		//new wxStaticText( this, -1, "Mapper:", wxPoint( 10, 55 ) );
		//new wxStaticText( this, -1, ( boost::format( "%1% (#%2%)" ) % g_mainboard->GetMemoryMapper()->GetName() % cartridge->GetMapperId() ).str().c_str(), wxPoint( 150, 55 ) );
		//new wxStaticText( this, -1, "16K PRG Pages:", wxPoint( 10, 70 ) );
		//new wxStaticText( this, -1, ( boost::format( "%1%" ) % cartridge->GetProgramPageCount() ).str().c_str(), wxPoint( 150, 70 ) );
		//new wxStaticText( this, -1, "8K CHR Pages:", wxPoint( 10, 85 ) );
		//new wxStaticText( this, -1, ( boost::format( "%1%" ) % cartridge->GetCharacterPageCount() ).str().c_str(), wxPoint( 150, 85 ) );
		//new wxStaticText( this, -1, "SRAM battery-backed:", wxPoint( 10, 100 ) );
		//new wxStaticText( this, -1, ( cartridge->IsSRamEnabled() ? "Yes" : "No" ), wxPoint( 150, 100 ) );
		//new wxStaticText( this, -1, "Trainer present:", wxPoint( 10, 115 ) );
		//new wxStaticText( this, -1, ( cartridge->HasTrainer() ? "Yes" : "No" ), wxPoint( 150, 115 ) );
	}
};


/////////////////////////////////////////////
// about dialog


class AboutDialog : public wxDialog
{
private:
	DECLARE_EVENT_TABLE();

public:
	AboutDialog( wxWindow* parent );

	void OnOKButton( wxCommandEvent& ev );

};


////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
	EVT_BUTTON(0,	AboutDialog::OnOKButton)
END_EVENT_TABLE()


AboutDialog::AboutDialog( wxWindow* parent )
	: wxDialog(parent, -1, "About", wxDefaultPosition)
{
	wxFlexGridSizer* mainBox = new wxFlexGridSizer(3, 1, 10, 10);

	mainBox->Add( new wxStaticText( this, -1, ( boost::wformat( L"%1% v%2%" ) % APP_NAME % APP_VERSION_STRING ).str().c_str() ), 0, wxALIGN_CENTER|wxALL, 10);

	wxTextCtrl* textctrl = new wxTextCtrl(this, -1, ( boost::wformat( L"Pete Ward, 2007\n\npeteward00@gmail.com" ) ).str().c_str(),
		wxDefaultPosition, wxSize(220, 70), wxTE_READONLY|wxTE_MULTILINE|wxCAPTION);
	mainBox->Add(textctrl, 0, wxALIGN_CENTER|wxALL, 10);

	wxBoxSizer* box2 = new wxBoxSizer(wxHORIZONTAL);

	wxButton* okButton = new wxButton(this, 0, "OK", wxDefaultPosition, wxDefaultSize, 0);
	box2->Add(okButton, 0, wxALIGN_CENTER|wxALL, 5);

	mainBox->Add(box2, 0, wxALIGN_CENTER|wxALL, 5);

	this->SetSizerAndFit(mainBox);
}


void AboutDialog::OnOKButton(wxCommandEvent& ev)
{
//	Processor65C816::OutputAllInstructions();
	EndModal( wxID_OK );
}


/////////////////////////////////////////////
// Bottom status bar for MainFrame


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
	SetStatusText( ( boost::wformat( L"FPS: %1$.2f" ) % fps ).str().c_str(), 1 );
}


void StatusBar::SetName( const std::wstring& name )
{
	SetStatusText( name, 0 );
}


/////////////////////////////////////////////


MainFrame::MainFrame( int windowWidth, int windowHeight )
: wxFrame( NULL, -1, APP_NAME ), canvas( NULL )
{
	// status bar
	statusBar = new StatusBar(this);
	SetStatusBar(statusBar);
	SetStatusBarPane( -1 );

	// set up menus
	menuBar = new wxMenuBar();

	// state menus
	saveStateMenu = new wxMenu();
	saveStateMenu->Append( ID_MENUSAVESTATE_0, L"Slot 1\tF5" );
	saveStateMenu->Append( ID_MENUSAVESTATE_1, L"Slot 2\tF6" );
	saveStateMenu->Append( ID_MENUSAVESTATE_2, L"Slot 3\tF7" );
	saveStateMenu->Append( ID_MENUSAVESTATE_3, L"Slot 4\tF8" );

	// for some reason using for loops to populate the menus cause the screen to be permanently black?!

//	for ( int i=0; i<4; ++i )
//		saveStateMenu->Append( ID_MENUSAVESTATE_0 + i, wxString( ( boost::format( "Slot %1%\tF%2%" ) % ( i + 1 ) % ( i + 5 ) ).str().c_str() ) );

	loadStateMenu = new wxMenu();
	loadStateMenu->Append( ID_MENULOADSTATE_0, L"Slot 1\tF9" );
	loadStateMenu->Append( ID_MENULOADSTATE_1, L"Slot 2\tF10" );
	loadStateMenu->Append( ID_MENULOADSTATE_2, L"Slot 3\tF11" );
	loadStateMenu->Append( ID_MENULOADSTATE_3, L"Slot 4\tF12" );

//	for ( int i=0; i<4; ++i )
//		loadStateMenu->Append( ID_MENULOADSTATE_0 + i, wxString( ( boost::format( "Slot %1%\tF%2%" ) % ( i + 1 ) % ( i + 9 ) ).str().c_str() ) );

	// File
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(ID_MENULOAD, L"Load...");
	fileMenu->AppendSeparator();
	fileMenu->AppendSubMenu( saveStateMenu, L"Save State" );
	fileMenu->AppendSubMenu( loadStateMenu, L"Load State" );
	fileMenu->AppendSeparator();

#ifdef ENABLE_DECOMPILE
	fileMenu->Append(ID_MENUDECOMPILE, L"Decompile...");
	fileMenu->AppendSeparator();
#endif

	fileMenu->Append(ID_MENUEXIT, L"Exit");

	menuBar->Append(fileMenu, L"File");

	// Options
	wxMenu* modeMenu = new wxMenu();
	menuNtsc = new wxMenuItem( modeMenu, ID_MENUNTSCMODE, L"NTSC", wxEmptyString, wxITEM_CHECK );
	modeMenu->Append( menuNtsc );
	menuNtsc->Check();
	menuPal = new wxMenuItem( modeMenu, ID_MENUPALMODE, L"PAL", wxEmptyString, wxITEM_CHECK );
	modeMenu->Append( menuPal );
	menuPal->Check( false );

	wxMenu* optionsMenu = new wxMenu();
	optionsMenu->Append( ID_MENUOPTIONS, L"Options..." );
	optionsMenu->AppendSeparator();
	optionsMenu->AppendSubMenu( modeMenu, L"Mode" );
	optionsMenu->AppendSeparator();
	optionsMenu->Append( ID_MENUCOLDRESET, L"Cold Reset" );
	optionsMenu->Append( ID_MENUWARMRESET, L"Warm Reset" );
	optionsMenu->AppendSeparator();
	optionsMenu->Append( ID_MENUCARTINFO, L"Cartridge Information" );

	menuBar->Append( optionsMenu, L"Options" );

	// size menu
	wxMenu* sizeMenu = new wxMenu();

	sizeMenu->Append( ID_MENUSIZE_0, L"256 x 240" );
	sizeMenu->Append( ID_MENUSIZE_1, L"512 x 480" );
	sizeMenu->Append( ID_MENUSIZE_2, L"768 x 720" );
	sizeMenu->Append( ID_MENUSIZE_3, L"1024 x 960" );

	menuBar->Append( sizeMenu, L"Size" );

	// speed menu
	wxMenu* speedMenu = new wxMenu();

	speedMenus[0] = new wxMenuItem( speedMenu, ID_MENUSPEED_0, L"Normal (100%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[0] );
	speedMenu->AppendSeparator();
	
	speedMenus[1] = new wxMenuItem( speedMenu, ID_MENUSPEED_1, L"Very fast (Unlimited)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[1] );
	speedMenus[2] = new wxMenuItem( speedMenu, ID_MENUSPEED_2, L"Fast (150%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[2] );
	speedMenus[3] = new wxMenuItem ( speedMenu, ID_MENUSPEED_3, L"A bit fast (125%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[3] );
	speedMenu->AppendSeparator();

	speedMenus[4] = new wxMenuItem( speedMenu, ID_MENUSPEED_4, L"A bit slow (75%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[4] );
	speedMenus[5] = new wxMenuItem( speedMenu, ID_MENUSPEED_5, L"Slow (50%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[5] );
	speedMenus[6] = new wxMenuItem( speedMenu, ID_MENUSPEED_6, L"Very slow (25%)", wxEmptyString, wxITEM_CHECK );
	speedMenu->Append( speedMenus[6] );

	menuBar->Append( speedMenu, L"Speed" );

	speedMenus[ g_options->ApplicationSpeed ]->Check();

#ifdef ENABLE_LOGGING
	wxMenu* logMenu = new wxMenu();

	logMenu->Append( ID_MENULOGALL, L"All" );
	logMenu->AppendSeparator();

	loggingMenus[0] = new wxMenuItem( logMenu, ID_MENULOGCPU, L"CPU", wxEmptyString, wxITEM_CHECK );
	loggingMenus[0]->Check( Log::IsTypeEnabled( LOG_CPU ) );
	loggingMenus[1] = new wxMenuItem( logMenu, ID_MENULOGPPU, L"PPU", wxEmptyString, wxITEM_CHECK );
	loggingMenus[1]->Check( Log::IsTypeEnabled( LOG_PPU ) );
	loggingMenus[2] = new wxMenuItem( logMenu, ID_MENULOGMAPPER, L"Mapper", wxEmptyString, wxITEM_CHECK );
	loggingMenus[2]->Check( Log::IsTypeEnabled( LOG_MAPPER ) );
	loggingMenus[3] = new wxMenuItem( logMenu, ID_MENULOGMISC, L"Misc", wxEmptyString, wxITEM_CHECK );
	loggingMenus[3]->Check( Log::IsTypeEnabled( LOG_MISC ) );

	for ( int i=0; i<4; ++i )
		logMenu->Append( loggingMenus[i] );

	menuBar->Append(logMenu, L"Logging");
#endif

	// Help
	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(ID_MENUABOUT, L"About...");

	menuBar->Append(helpMenu, L"Help");

	this->SetMenuBar(menuBar);

	// drop target
	this->SetDropTarget( new FileDropTarget( this ) );

	// accelerator table (for save states)
	accelEntries[0].Set( wxACCEL_NORMAL, WXK_F5, ID_MENUSAVESTATE_0 );
	accelEntries[1].Set( wxACCEL_NORMAL, WXK_F6, ID_MENUSAVESTATE_1 );
	accelEntries[2].Set( wxACCEL_NORMAL, WXK_F7, ID_MENUSAVESTATE_2 );
	accelEntries[3].Set( wxACCEL_NORMAL, WXK_F8, ID_MENUSAVESTATE_3 );
	accelEntries[4].Set( wxACCEL_NORMAL, WXK_F9, ID_MENULOADSTATE_0 );
	accelEntries[5].Set( wxACCEL_NORMAL, WXK_F10, ID_MENULOADSTATE_1 );
	accelEntries[6].Set( wxACCEL_NORMAL, WXK_F11, ID_MENULOADSTATE_2 );
	accelEntries[7].Set( wxACCEL_NORMAL, WXK_F12, ID_MENULOADSTATE_3 );
	accelerators = wxAcceleratorTable( AcceleratorEntries, accelEntries );
	SetAcceleratorTable( accelerators );

	// set up opengl
	wxGLAttributes attributes;
	attributes.PlatformDefaults()
		.Depth(0)
		.BufferSize(24)
		.DoubleBuffer()
		.EndList();
	this->canvas = new GLCanvas( this, wxSUNKEN_BORDER, attributes );

	this->SetClientSize( windowWidth, windowHeight );
}


MainFrame::~MainFrame()
{
	delete this->canvas;
}


void MainFrame::OnQuit( wxCommandEvent& ev )
{
	Close( TRUE );
}


void MainFrame::Run()
{
	this->canvas->SetZoomSize( GetClientSize().GetWidth(), GetClientSize().GetHeight() );
	this->Show(TRUE);
}


void MainFrame::OnMenuSpeed( wxCommandEvent& ev )
{
	speedMenus[ g_options->ApplicationSpeed ]->Check( false );
	g_options->ApplicationSpeed = ev.GetId() - ID_MENUSPEED_0;

	if ( g_activeMainboard != NULL )
		g_activeMainboard->OnChangeSpeed();
}


void MainFrame::OnMenuCartInfo( wxCommandEvent& ev )
{
	if ( g_activeMainboard && g_activeMainboard->IsRunning() )
	{
		//CartridgeInfo* info = new CartridgeInfo( this, g_activeMainboard->GetCartridge() );
		//info->ShowModal();
	}
}


void MainFrame::OnMenuLoad( wxCommandEvent& ev )
{
	wxFileDialog* dialog = new wxFileDialog( this, L"Open iNES ROM file", wxEmptyString, wxEmptyString, L"iNES files (*.nes)|*.nes|All files (*.*)|*.*",
		wxFD_FILE_MUST_EXIST | wxFD_OPEN );

	if ( dialog->ShowModal() == wxID_OK )
	{
		try
		{
			wxGetApp().LoadRomFile( dialog->GetPath().ToStdWstring() );
		}
		catch ( std::exception& e )
		{
			NesulatorApp::DisplayError( e.what() );
		}
	}
}


void MainFrame::OnMenuDecompile( wxCommandEvent& ev )
{
#ifdef ENABLE_DECOMPILE
	wxFileDialog* dialog = new wxFileDialog( this, L"Open iNES ROM file", wxEmptyString, wxEmptyString, L"iNES files (*.nes)|*.nes|All files (*.*)|*.*",
		wxFD_FILE_MUST_EXIST | wxFD_OPEN );

	if ( dialog->ShowModal() == wxID_OK )
	{
		try
		{
			wxFileDialog* decompileDialog = new wxFileDialog( this, L"Save to text file", wxEmptyString, wxEmptyString,
				L"Text file (*.txt)|*.txt|All files (*.*)|*.*",
				wxFD_SAVE );

			if ( decompileDialog->ShowModal() == wxID_OK )
			{
				Decompiler decompiler;
				decompiler.Save( dialog->GetPath().ToStdWstring(), decompileDialog->GetPath().ToStdWstring() );
			}
		}
		catch ( std::exception& e )
		{
			NesulatorApp::DisplayError( e.what() );
		}
	}
#endif
}


void MainFrame::OnMenuLogging( wxCommandEvent& ev )
{
	switch ( ev.GetId() )
	{
	case ID_MENULOGALL:
	{
		const bool allenabled = Log::IsTypeEnabled( LOG_CPU | LOG_PPU | LOG_MAPPER | LOG_MISC );

		if ( !allenabled )
			Log::EnableType( LOG_CPU | LOG_PPU | LOG_MAPPER | LOG_MISC );
		else
			Log::DisableType( LOG_CPU | LOG_PPU | LOG_MAPPER | LOG_MISC );
			
		for ( int i=0; i<4; ++i )
			loggingMenus[i]->Check( !allenabled );
	}
		break;
	case ID_MENULOGCPU:
		Log::ToggleEnabled( LOG_CPU );
		break;
	case ID_MENULOGPPU:
		Log::ToggleEnabled( LOG_PPU );
		break;
	case ID_MENULOGMAPPER:
		Log::ToggleEnabled( LOG_MAPPER );
		break;
	case ID_MENULOGMISC:
		Log::ToggleEnabled( LOG_MISC );
		break;
	}
}


void MainFrame::OnMenuExit( wxCommandEvent& ev )
{
	Close( TRUE );
}


void MainFrame::OnMenuAbout( wxCommandEvent& ev )
{
	Processor6502::OutputAllInstructions();

	AboutDialog* about = new AboutDialog( this );
	about->ShowModal();
}


void MainFrame::OnMenuOptions( wxCommandEvent& ev )
{
	OptionsDialog* optionsDialog = new OptionsDialog( this );
	optionsDialog->ShowModal();
}


void MainFrame::OnMenuColdReset( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::Reset, _1, true ) );
	}
}


void MainFrame::OnMenuWarmReset( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::Reset, _1, false ) );
	}
}


void MainFrame::OnSize( wxSizeEvent& ev )
{
	if ( canvas )
	{
		this->canvas->SetSize( wxSize( this->GetClientSize().GetWidth(), this->GetClientSize().GetHeight() ) );
		g_options->WindowWidth = this->GetClientSize().GetWidth();
		g_options->WindowHeight = this->GetClientSize().GetHeight();
	}
}


std::wstring MainFrame::GetSaveStateFilename( int slot ) const
{
	if ( g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		wxFileName path( g_options->SaveStatesDirectory + "\\" );

		path.MakeAbsolute();

		if ( !path.DirExists() )
			path.Mkdir();

		return ( boost::wformat( L"%1%%2$X_%3%.savestate" ) % path.GetPathWithSep().c_str() % g_activeMainboard->GetCartridge()->GetCRC32() % slot ).str();
	}
	else
		throw std::runtime_error( "Could not determine save state filename" );
}


void MainFrame::OnMenuLoadState( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		try
		{
			wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::LoadState, _1, GetSaveStateFilename( ev.GetId() - ID_MENULOADSTATE_0 + 1 ) ) );
		}
		catch ( std::exception& e )
		{
			NesulatorApp::DisplayError( e.what() );
		}
	}
}


void MainFrame::OnMenuSaveState( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		try
		{
			wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::SaveState, _1, GetSaveStateFilename( ev.GetId() - ID_MENUSAVESTATE_0 + 1 ) ) );
		}
		catch ( std::exception& e )
		{
			NesulatorApp::DisplayError( e.what() );
		}
	}
}


void MainFrame::OnMenuNTSCMode( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->GetTerritoryMode() != TERRITORY_NTSC )
	{
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::SetTerritoryMode, _1, TERRITORY_NTSC ) );
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::Reset, _1, true ) );
		menuPal->Check( false );
	}
}


void MainFrame::OnMenuPALMode( wxCommandEvent& ev )
{
	if ( g_activeMainboard != NULL && g_activeMainboard->GetTerritoryMode() != TERRITORY_PAL )
	{
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::SetTerritoryMode, _1, TERRITORY_PAL ) );
		wxGetApp().ConnectToReadySignal( boost::bind( &IMainBoard::Reset, _1, true ) );
		menuNtsc->Check( false );
	}
}


void MainFrame::OnMenuSize( wxCommandEvent& ev )
{
	switch ( ev.GetId() )
	{
	case ID_MENUSIZE_0:
		SetClientSize( 256, 240 );
		break;
	case ID_MENUSIZE_1:
		SetClientSize( 512, 480 );
		break;
	case ID_MENUSIZE_2:
		SetClientSize( 768, 720 );
		break;
	case ID_MENUSIZE_3:
		SetClientSize( 1024, 960 );
		break;
	case ID_MENUSIZE_FULLSCREEN:
	{
	}
		break;
	}
}


void MainFrame::OnKeyUp(wxKeyEvent& ev)
{
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnKeyUp( ev.GetKeyCode() );
}


void MainFrame::OnKeyDown(wxKeyEvent& ev)
{
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnKeyDown( ev.GetKeyCode() );
}


void MainFrame::OnMouseLeftDown( wxMouseEvent& ev )
{
	int x, y;
	canvas->CoordinatesToCanvas( ev.GetX(), ev.GetY(), x, y );
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnMouseLeftDown( x, y );
	ev.Skip();
}


void MainFrame::OnMouseLeftUp( wxMouseEvent& ev )
{
	int x, y;
	canvas->CoordinatesToCanvas( ev.GetX(), ev.GetY(), x, y );
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnMouseLeftUp( x, y );
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	NesulatorApp::AppQuit();
}