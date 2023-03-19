
#include "../main.h"
#include "app.h"

#include "../mainboard.h"

#include "wx/evtloop.h"
#include "wx/filename.h"
#include <boost/bind/bind.hpp>


IMainBoard* g_activeMainboard = NULL;


namespace
{
	// EventLoop class -
	// required by wxWidgets to override event management
	class EventLoop : public wxEventLoop
	{
	public:
		EventLoop( NesulatorApp *app );
		virtual void OnExit();

	private:
		NesulatorApp *mapp;

	};


	EventLoop::EventLoop( NesulatorApp *app ) : wxEventLoop()
	{
		mapp = app;
	}


	void EventLoop::OnExit()
	{
		mapp->StartShutdown();
	}
};


////////////////////////////////////////////////////////////////////////////////////


void NesulatorApp::DisplayError( const std::string& error )
{
	::wxMessageBox( error, "Error", 4|wxCENTRE );
}


std::wstring NesulatorApp::GetApplicationPathWithSep()
{
	wxFileName path( wxTheApp->argv[0] );
	path.MakeAbsolute();
	return path.GetPathWithSep().ToStdWstring();
}


std::wstring NesulatorApp::GetConfigFilename()
{
	return ( boost::wformat( L"%1%%2%" ) % GetApplicationPathWithSep() % L"config.xml" ).str();
}

////////////////////////////////////////////////////////////////////////////////////


NesulatorApp::NesulatorApp()
: mainFrame( NULL ), mQuitNow( false )
{
}


void NesulatorApp::StartShutdown()
{
	mQuitNow = true;
	if ( g_activeMainboard != NULL )
		g_activeMainboard->StopLoop();
}


bool NesulatorApp::OnInit()
{
	try
	{
		//if ( SDL_Init( SDL_INIT_AUDIO ) < 0 )
		//	return false;

		g_options = new Options();
		g_options->LoadFromFile( GetConfigFilename() );

		g_nesMainboard = new MainBoard( this );
		g_nesMainboard->CreateComponents();

		mainFrame = new MainFrame( g_options->WindowWidth, g_options->WindowHeight );

		Log::Open( mainFrame );
		Log::Enable( true );
		Log::EnableType( LOG_ERROR | LOG_MISC );

		mainFrame->Run();
	}
	catch ( std::exception& e )
	{
		DisplayError( e.what() );
		return false;
	}

	return true;
}


int NesulatorApp::OnExit()
{
	try
	{
		if ( g_nesMainboard != NULL )
		{
			g_nesMainboard->Shutdown();
			delete g_nesMainboard;
			g_nesMainboard = NULL;
		}

		if ( g_options != NULL )
		{
			g_options->SaveToFile( GetConfigFilename() );
			delete g_options;
			g_options = NULL;
		}
		
		//SDL_Quit();
		Log::Close();
	}
	catch ( std::exception& e )
	{
		DisplayError( e.what() );
	}

	return wxApp::OnExit();
}


int NesulatorApp::OnRun()
{
	try
	{
		SetExitOnFrameDelete(true);
		stopWatch.Start();

		EventLoop eventLoop(this);
		wxEventLoop::SetActive(&eventLoop);

		m_mainLoop = &eventLoop;

		while ( !mQuitNow )
		{
			if ( g_activeMainboard != NULL )
			{
				g_activeMainboard->DoLoop();
				break;
			}
			else
			{
				while ( g_activeMainboard == NULL && !mQuitNow )
				{
					Sleep( 100 );
					if (!mQuitNow) {
						DoWindowsEvents();
					}
				}
			}
		}
	}
	catch ( std::exception& e )
	{
		DisplayError( e.what() );
	}

	m_mainLoop = NULL;
	return 0;
}


//////////////////////
// MainBoard callback interface


void NesulatorApp::SetFps( float fps )
{
	if ( g_activeMainboard != NULL && !g_activeMainboard->IsRunning() )
		return;
	mainFrame->SetFps( fps );
}


bool NesulatorApp::DoWindowsEvents()
{
	//if ( g_activeMainboard != NULL && !g_activeMainboard->IsRunning() )
	//	return false;

	while ( !mQuitNow && !Pending() && ProcessIdle() );

	while ( Pending() )
	{
		if ( !Dispatch() )
		{
			mQuitNow = true;
			break;
		}
	}

	if ( mQuitNow )
	{
		while (Pending())
			Dispatch(); // dispatch final waiting messages
		m_mainLoop = NULL;
	}

	return !mQuitNow;
}


void NesulatorApp::Render()
{
	if ( !mQuitNow && g_activeMainboard != NULL && g_activeMainboard->IsRunning() )
	{
		mainFrame->GetCanvas()->Render();
	}
}


void NesulatorApp::Sleep( int milliSeconds )
{
	::wxMilliSleep( milliSeconds );
}


int NesulatorApp::GetTicks()
{
	return stopWatch.Time();
}


void NesulatorApp::OnCartridgeLoad( IMainBoard* mainBoard )
{
	g_activeMainboard = mainBoard;

	if ( g_activeMainboard != NULL )
	{
		wxGetApp().GetMainFrame()->GetCanvas()->AttachRenderBuffer( g_activeMainboard->GetRenderBuffer() );
		mainFrame->SetRomName( g_activeMainboard->GetCartridge()->GetName() );
	}
}


void NesulatorApp::OnStopRunning( IMainBoard* mainBoard )
{
}


void NesulatorApp::LoadRomFile( const std::wstring& filename )
{
	try
	{
		size_t indexPos = filename.find_last_of( '.' );
		if ( indexPos == std::string::npos )
			throw std::runtime_error( "Invalid file extension for file" );

		std::string extension;
		std::transform( filename.begin() + indexPos + 1, filename.end(), std::back_inserter( extension ), tolower );

		IMainBoard* mainboard = NULL;

		if ( extension == "nes" )
		{
			mainboard = g_nesMainboard;
		}
		else
			throw std::runtime_error( "Invalid file extension for file" );

		assert( mainboard != NULL );

		if ( g_activeMainboard != NULL )
		{
			g_activeMainboard->StopLoop();
			g_activeMainboard = mainboard;
			// load cartridge when active mainboard has reported ready message (ie fully stopped)
			ReadyState.connect( boost::bind( &IMainBoard::LoadCartridge, mainboard, filename ) );
		}
		else
		{
			// load cartridge immediately if no active mainboard
			g_activeMainboard = mainboard;
			g_activeMainboard->LoadCartridge( filename );
		}
	}
	catch ( std::exception& )
	{
		g_activeMainboard = NULL;
		throw;
	}
}


void NesulatorApp::OnReadyCommand( IMainBoard* mainBoard )
{
	ReadyState( mainBoard );
	ReadyState.disconnect_all_slots();
}


//////////////////////


IMPLEMENT_APP(NesulatorApp)
