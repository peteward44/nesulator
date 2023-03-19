

#include "main.h"
#include "mainboard.h"
#include <sstream>
#include <cmath>
#include <boost/bind/bind.hpp>
#include "decompiler.h"
#include <fstream>
#include "log.h"
#include "wx/wx.h"
#include "wx/filename.h"

MainBoard* g_nesMainboard = NULL;
bool Log::enable = false;
int Log::typesEnabled = 0;

//#pragma comment( lib, "sdl.lib" )
//#pragma comment( lib, "sdl_sound_static.lib" )

const wchar_t* APP_NAME = L"Nesulator";
const wchar_t* APP_VERSION_STRING = L"0.2";


/////////////////////////////////////////////////////////////////////////////////


MainBoard::MainBoard( MainBoardCallbackInterface* callbackInterface )
: callbackInterface( callbackInterface )
{
	modeConstants = NULL;
	SetTerritoryMode( TERRITORY_NTSC );

	targetTicks = ticksThisSecond = ticksLastFrame = ticksIncrement = 0;

	paused = stepFrame = takeScreenshot = false;

	frameCount = 0;

	running = false;
	synchroniser = NULL;
	apu = NULL;
	renderBuffer = NULL;
	ppu = NULL;
	cpumemory = NULL;
	processor = NULL;

	OnChangeSpeed();
}


MainBoard::~MainBoard()
{
	Shutdown();

	delete modeConstants;
	modeConstants = NULL;
}


void MainBoard::Shutdown()
{
	delete apu;
	apu = NULL;
	delete renderBuffer;
	renderBuffer = NULL;
	delete cpumemory;
	cpumemory = NULL;
	delete ppu;
	ppu = NULL;
	delete processor;
	processor = NULL;
	delete synchroniser;
	synchroniser = NULL;
}


void MainBoard::CreateComponents()
{
	synchroniser = new Synchroniser();
	cpumemory = new CPUMemory();
	ppu = new PictureProcessingUnit();
	apu = new APU();
	processor = new Processor6502();
	renderBuffer = new RenderBuffer();

	inputDeviceBus.SyncWithOptions();
}


void MainBoard::Reset( bool cold )
{
	Log::Write( LOG_MISC, ( boost::wformat( L"%1% reset" ) % ( cold ? L"Cold" : L"Warm" ) ).str() );
	ResetEvent( cold );
	renderBuffer->ClearBuffer( ppu->GetBackgroundPaletteIndex() );
}


void MainBoard::OnChangeSpeed()
{
	float multiplier;

	switch ( g_options->ApplicationSpeed )
	{
	default:
	case 0:
		multiplier = 1.0f;
		break;
	case 1:
		multiplier = 1000.0f; // unlimited
		break;
	case 2:
		multiplier = 1.5f; // 150%
		break;
	case 3:
		multiplier = 1.25f; // 125%
		break;
	case 4:
		multiplier = 0.75f; // 75%
		break;
	case 5:
		multiplier = 0.50f; // 50%
		break;
	case 6:
		multiplier = 0.25f; // 25%
		break;
	}

	ticksIncrement = (boost::uint32_t)(( 1000.0f / ( ( (float)this->modeConstants->RefreshRate() * multiplier )) ));
}


bool MainBoard::StartNewFrame( )
{
	if ( !running )
	{
		return false;
	}

	if ( !callbackInterface->DoWindowsEvents() )
	{
		running = false;
		return false;
	}

	callbackInterface->Render();

	bool waiting = true;
	boost::uint32_t ticksThisFrame = 0;

	while ( paused )
	{
		callbackInterface->Sleep( 100 );
		if ( !callbackInterface->DoWindowsEvents() )
		{
			running = false;
			return false;
		}

		if ( stepFrame )
		{
			stepFrame = false;
			break;
		}
	}

	while ( waiting )
	{
		ticksThisFrame = callbackInterface->GetTicks();

		if ( ticksThisFrame > targetTicks
			/* || (g_options->SoundEnabled && g_options->ApplicationSpeed == 0)*/) // allow the sound library to handle setting the framerate as it is more accurate
		{
			waiting = false;
		}
		else
			callbackInterface->Sleep( 0 );
	}

	targetTicks = (boost::uint32_t)ticksThisFrame + ticksIncrement;

	boost::uint32_t ticksDifference = ticksThisFrame - ticksLastFrame;
	ticksLastFrame = ticksThisFrame;
	ticksThisSecond += ticksDifference;

	frameCount++;

	if ( ticksThisSecond >= 1000 )
	{
		float fps = (float)frameCount / ((float)ticksThisSecond / 1000.0f );

		callbackInterface->SetFps( fps );

		ticksThisSecond = 0;
		frameCount = 0;
	}

	if ( takeScreenshot )
	{
		std::wstring screenshotFilename = GetScreenshotFilename();

		int x = 0, y = 0;
		renderBuffer->GetBufferSize( x, y );

		wxImage image( x, y, (unsigned char*)renderBuffer->GetRGBBuffer( false ), true );

		image.Rescale( x * 2, y * 2, wxIMAGE_QUALITY_HIGH );
		image.SaveFile( screenshotFilename.c_str(), wxBITMAP_TYPE_BMP );

		image.Destroy();

		takeScreenshot = false;

		Log::Write( LOG_MISC, ( boost::wformat( L"Screenshot saved: %1%" ) % screenshotFilename ).str() );
	}

	// start next frame
	renderBuffer->ClearBuffer( ppu->GetBackgroundPaletteIndex() );

	return true;
}


std::wstring MainBoard::GetScreenshotFilename()
{
	wxFileName path( g_options->ScreenshotsDirectory + "\\" );

	if ( !path.DirExists() )
		path.Mkdir();

	wxFileName file;
	int num = 1;
	do
	{
		file = wxFileName( ( boost::wformat( L"%1%%2$X_%3%.bmp" ) % path.GetPathWithSep().c_str() % GetCartridge()->GetCRC32() % num ).str() );
		num++;
	}
	while ( file.FileExists() );
	return file.GetFullPath().ToStdWstring();
}


void MainBoard::DoLoop()
{
	running = true;

	ticksLastFrame = callbackInterface->GetTicks();

	Reset( true ); // always start with a cold reset

	while ( running )
	{
		synchroniser->RunCycle();
		callbackInterface->OnReadyCommand( this );
	}
}


void MainBoard::StopLoop()
{
	running = false;
}


void MainBoard::LoadCartridge( const std::wstring& path )
{
	try
	{
		CartridgePtr_t cartridge = CreateCartridgeFromROM( path, false );
		this->cartridge = cartridge;
		callbackInterface->OnCartridgeLoad( this );
	}
	catch ( std::exception& e )
	{
		::wxMessageBox( L"Error loading ROM", L"Error loading ROM", 4 | wxCENTRE);
	}
}


//#include "boost/filesystem/operations.hpp"
//
//
//
//void CopyMapperRoms( int mapper, const std::string& directory, const std::string& outdir, bool recursive )
//{
//	boost::filesystem::path basepath( directory, boost::filesystem::native );
//	boost::filesystem::path outpath( outdir, boost::filesystem::native );
//
//	boost::filesystem::directory_iterator dirbeg( basepath );
//	boost::filesystem::directory_iterator dirend;
//
//	for (; dirbeg != dirend; ++dirbeg )
//	{
//		boost::filesystem::path path = *dirbeg;
//
//		if ( boost::filesystem::is_directory( path ) )
//		{
//			if ( recursive )
//			{
//				boost::filesystem::path newoutdir( outpath.string() + "/" + path.leaf(), boost::filesystem::native );
//				boost::filesystem::create_directory( newoutdir );
//				
//				CopyMapperRoms( mapper, path.string(), newoutdir.string(), true );
//			}
//		}
//		else
//		{
//			try
//			{
//				CartridgePtr_t cartridge = CreateCartridgeFromROM( path.string(), true );
//				if ( cartridge->GetMapperId() == mapper )
//				{
//					boost::filesystem::copy_file( path, boost::filesystem::path( outpath.string() + "/" + path.leaf(), boost::filesystem::native ) );
//				}
//			}
//			catch (...)
//			{
//			}
//		}
//	}
//}


void MainBoard::SetTerritoryMode( TERRITORY_MODE mode )
{
	delete modeConstants;

	switch ( mode )
	{
	case TERRITORY_NTSC:
		modeConstants = new NTSCModeConstants();
		break;
	case TERRITORY_PAL:
		modeConstants = new PALModeConstants();
		break;
	default:
		throw std::runtime_error( ( boost::format( "Unknown mode specified: %1%" ) % mode ).str() );
	}

	OnChangeSpeed(); // refresh rate has changed, recalculate frame timing

	Log::Write( LOG_MISC, ( boost::wformat( L"Changing to mode: %1%" ) % modeConstants->Name() ).str() );
}


void MainBoard::SaveState( const std::wstring& filename )
{
	if ( !running )
		return;

	std::ofstream ostr( filename.c_str(), std::ios::binary );

	// write save state header
	ostr.write( "NSS\0", 4 );

	// write cartridge information
	WriteType< boost::uint32_t >( ostr, cartridge->GetCRC32() );

	cpumemory->SaveState( ostr );
	inputDeviceBus.SaveState( ostr );
	processor->SaveState( ostr );
	ppu->SaveState( ostr );
	synchroniser->SaveState( ostr );
	apu->SaveState( ostr );
	GetMemoryMapper()->SaveState( ostr );

	ostr.close();

	Log::Write( LOG_MISC, ( boost::wformat( L"State saved to: %1%" ) % filename ).str() );
}


void MainBoard::LoadState( const std::wstring& filename )
{
	if ( !running )
		return;

	std::ifstream istr( filename.c_str(), std::ios::binary );

	if ( !istr.is_open() )
		return;

	// write save state header
	unsigned int magicNumber = ReadType< boost::uint32_t >( istr );
	if ( magicNumber != 0x0053534E ) // string 'NSS'
		throw std::runtime_error( "Not a valid Nesulator save state file" );

	// write cartridge information
	boost::uint32_t crc = ReadType< boost::uint32_t >( istr );

	if ( crc != cartridge->GetCRC32() )
		throw std::runtime_error( "CRC does not match" );

	cpumemory->LoadState( istr );
	inputDeviceBus.LoadState( istr );
	processor->LoadState( istr );
	ppu->LoadState( istr );
	synchroniser->LoadState( istr );
	apu->LoadState( istr );
	GetMemoryMapper()->LoadState( istr );

	istr.close();

	Log::Write( LOG_MISC, ( boost::wformat( L"State loaded: %1%" ) % filename ).str() );
}
