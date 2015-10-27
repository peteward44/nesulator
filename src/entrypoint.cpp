
#include "stdafx.h"
#include "mainboard.h"
#include <iostream>
#include "decompiler.h"


#pragma comment(lib, "sdl.lib")
//#pragma comment(lib, "sdlmain.lib")
#pragma comment( lib, "sdl_sound_static.lib" )

#include <windows.h>


const char* APP_NAME = "Nesulator";
const char* APP_VERSION_STRING = "0.1a";


//void InitSDL()
//{
//	if ( SDL_Init( /* SDL_INIT_VIDEO | */ SDL_INIT_AUDIO ) < 0 )
//		throw std::exception( "SDL_Init failed" );
//}
//
//
//void QuitSDL()
//{
//	SDL_Quit();
//}



//int __stdcall WinMain (HINSTANCE hThisInstance,
//                    HINSTANCE hPrevInstance,
//                    LPSTR lpszArgument,
//                    int nFunsterStil)
//{
//	bool copyroms = false;
//
//	MainBoard& mb = MainBoard::Instance();
//
//	try
//	{
//		Log::Open();
//		Log::Enable( true );
//
//		InitSDL();
//
//		if ( copyroms )
//		{
//			CopyMapperRoms( 0, "X:\\games\\nes_roms\\NESRen\\", "d:\\mapper0roms\\", true );
//		}
//		else
//		{
//			JoystickKeyMapPtr_t keyMap1 = JoystickKeyMap::CreateKeyMap();
//
//			keyMap1->SetKey( SDLK_SPACE, JS_START );
//			keyMap1->SetKey( SDLK_z, JS_SELECT );
//			keyMap1->SetKey( SDLK_x, JS_A );
//			keyMap1->SetKey( SDLK_c, JS_B );
//			keyMap1->SetKey( SDLK_UP, JS_UP );
//			keyMap1->SetKey( SDLK_DOWN, JS_DOWN );
//			keyMap1->SetKey( SDLK_LEFT, JS_LEFT );
//			keyMap1->SetKey( SDLK_RIGHT, JS_RIGHT );
//
//			mb.ConnectJoystick( 0, keyMap1 );
//
//			mb.Run();
//		}
//	}
//	catch (std::exception& e)
//	{
//		Log::Write( ( boost::format( "Exception caught: %1%" ) % e.what() ).str() );
//		ShowMessageBox( ( boost::format( "%1%" ) % e.what() ).str(), "Fatal error" );
//	}
//
//	mb.Shutdown();
//
//	QuitSDL();
//	Log::Close();
//
//	return 0;
//}
