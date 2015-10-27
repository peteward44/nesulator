
#include "stdafx.h"
#include "gui/app.h"
#include "main.h"
#include "options.h"
#include "tinyxml/tinyxml.h"
#include "wx/filename.h"
#include "gui/app.h"

Options* g_options = NULL;



Options::Options()
{
	ApplicationSpeed = 0;
	AllowMoreThanEightSprites = false;
	UseNtscFiltering = false;

	SoundEnabled = SOUND_ENABLED_BY_DEFAULT;
	SoundVolume = 0;

	inputDevices[0] = INPUT_JOYPAD;
	inputDevices[1] = INPUT_ZAPPER;

	WindowWidth = 512;
	WindowHeight = 480;

	SaveStatesDirectory = NesulatorApp::GetApplicationPathWithSep() + "savestates";
	ScreenshotsDirectory = NesulatorApp::GetApplicationPathWithSep() + "screenshots";

	JoypadKeyMap& keymap = GetJoypadKeyMap( 0 );

	keymap.SetKey( WXK_SPACE, JS_START );
	keymap.SetKey( 'Z', JS_SELECT );
	keymap.SetKey( 'X', JS_A );
	keymap.SetKey( 'C', JS_B );
	keymap.SetKey( WXK_UP, JS_UP );
	keymap.SetKey( WXK_DOWN, JS_DOWN );
	keymap.SetKey( WXK_LEFT, JS_LEFT );
	keymap.SetKey( WXK_RIGHT, JS_RIGHT );

	JoypadKeyMap& keymap2 = GetJoypadKeyMap( 1 );

	keymap2.SetKey( 'G', JS_START );
	keymap2.SetKey( 'H', JS_SELECT );
	keymap2.SetKey( 'Y', JS_A );
	keymap2.SetKey( 'U', JS_B );
	keymap2.SetKey( 'I', JS_UP );
	keymap2.SetKey( 'K', JS_DOWN );
	keymap2.SetKey( 'J', JS_LEFT );
	keymap2.SetKey( 'L', JS_RIGHT );
}


void Options::LoadFromFile( const std::string& filename )
{
	TiXmlDocument doc;

	if ( !doc.LoadFile( filename.c_str() ) )
		return;

	for( TiXmlNode* node = doc.FirstChild(); node != NULL; node = node->NextSibling() )
	{
		TiXmlElement* element = node->ToElement();
		if ( element != NULL )
		{
			const char* name = element->Value();
			if ( _stricmp( name, "Sound" ) == 0 )
			{
				int enabled = 0;
				element->QueryIntAttribute( "Enabled", &enabled );
				SoundEnabled = enabled > 0;
				element->QueryIntAttribute( "Volume", &SoundVolume );
			}
			else if ( _stricmp( name, "Window" ) == 0 )
			{
				element->QueryIntAttribute( "Width", &WindowWidth );
				element->QueryIntAttribute( "Height", &WindowHeight );
			}
			else if ( _stricmp( name, "KeyMap" ) == 0 )
			{
				int port = 0;
				element->QueryIntAttribute( "Port", &port );

				JoypadKeyMap& keymap = GetJoypadKeyMap( port );

				for ( int button=0; button<JS_BUTTON_COUNT; ++button )
				{
					char* buttonName = NULL;
					switch ( button )
					{
						case JS_A:
							buttonName = "A";
							break;
						case JS_B:
							buttonName = "B";
							break;
						case JS_SELECT:
							buttonName = "Select";
							break;
						case JS_START:
							buttonName = "Start";
							break;
						case JS_UP:
							buttonName = "Up";
							break;
						case JS_DOWN:
							buttonName = "Down";
							break;
						case JS_LEFT:
							buttonName = "Left";
							break;
						case JS_RIGHT:
							buttonName = "Right";
							break;
					}

					if ( buttonName != NULL )
					{
						int keyCode = -1;
						element->QueryIntAttribute( buttonName, &keyCode );
						keymap.SetKey( keyCode, (JOYSTICK_BUTTON)button );
					}
				}
			}
			else if ( _stricmp( name, "Device" ) == 0 )
			{
				int port = 0;
				element->QueryIntAttribute( "Port", &port );

				int deviceType = 0;
				element->QueryIntAttribute( "Type", &deviceType );

				inputDevices[ port ] = (INPUT_DEVICE)deviceType;
			}
			else if ( _stricmp( name, "Directories" ) == 0 )
			{
				const char* ss = element->Attribute( "SaveStates" );
				if ( ss != NULL )
					SaveStatesDirectory = ss;

				const char* ss2 = element->Attribute( "Screenshots" );
				if ( ss2 != NULL )
					ScreenshotsDirectory = ss2;
			}
			else if ( _stricmp( name, "Video" ) == 0 )
			{
				int allowMoreSprites = 0;
				element->QueryIntAttribute( "AllowMoreThanEightSprites", &allowMoreSprites );
				AllowMoreThanEightSprites = allowMoreSprites > 0;

				int ntscFiltering = 0;
				element->QueryIntAttribute( "UseNtscFiltering", &ntscFiltering );
				UseNtscFiltering = ntscFiltering > 0;
			}
		}
	}
}


void Options::SaveToFile( const std::string& filename )
{
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
	
	TiXmlElement* soundElement = new TiXmlElement( "Sound" );
	soundElement->SetAttribute( "Enabled", SoundEnabled ? 1 : 0 );
	soundElement->SetAttribute( "Volume", SoundVolume );
	doc.LinkEndChild( soundElement );
	
	TiXmlElement* windowElement = new TiXmlElement( "Window" );
	windowElement->SetAttribute( "Width", WindowWidth );
	windowElement->SetAttribute( "Height", WindowHeight );
	doc.LinkEndChild( windowElement );

	for ( int i=0; i<2; ++i )
	{
		TiXmlElement* keymapElement = new TiXmlElement( "KeyMap" );

		keymapElement->SetAttribute( "Port", i );

		JoypadKeyMap& keymap = GetJoypadKeyMap( i );

		keymapElement->SetAttribute( "Start", keymap.ButtonToKeyCode( JS_START ) );
		keymapElement->SetAttribute( "Select", keymap.ButtonToKeyCode( JS_SELECT ) );
		keymapElement->SetAttribute( "A", keymap.ButtonToKeyCode( JS_A ) );
		keymapElement->SetAttribute( "B", keymap.ButtonToKeyCode( JS_B ) );
		keymapElement->SetAttribute( "Up", keymap.ButtonToKeyCode( JS_UP ) );
		keymapElement->SetAttribute( "Down", keymap.ButtonToKeyCode( JS_DOWN ) );
		keymapElement->SetAttribute( "Left", keymap.ButtonToKeyCode( JS_LEFT ) );
		keymapElement->SetAttribute( "Right", keymap.ButtonToKeyCode( JS_RIGHT ) );
		
		doc.LinkEndChild( keymapElement );
	}

	for ( int i=0; i<2; ++i )
	{
		TiXmlElement* deviceElement = new TiXmlElement( "Device" );

		deviceElement->SetAttribute( "Port", i );
		deviceElement->SetAttribute( "Type", inputDevices[i] );
		
		doc.LinkEndChild( deviceElement );
	}

	TiXmlElement* dirElement = new TiXmlElement( "Directories" );
	dirElement->SetAttribute( "SaveStates", SaveStatesDirectory.c_str() );
	dirElement->SetAttribute( "Screenshots", ScreenshotsDirectory.c_str() );
	doc.LinkEndChild( dirElement );

	TiXmlElement* videoElement = new TiXmlElement( "Video" );
	videoElement->SetAttribute( "AllowMoreThanEightSprites", AllowMoreThanEightSprites ? 1 : 0 );
	videoElement->SetAttribute( "UseNtscFiltering", UseNtscFiltering ? 1 : 0 );
	doc.LinkEndChild( videoElement );

	doc.SaveFile( filename.c_str() );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////


JoypadKeyMap::JoypadKeyMap()
{
	memset( keys, -1, JS_BUTTON_COUNT * sizeof(int) );
}


JoypadKeyMap::JoypadKeyMap( const JoypadKeyMap& jkm )
{
	memcpy( keys, jkm.keys, JS_BUTTON_COUNT * sizeof(int) );
}


JoypadKeyMap& JoypadKeyMap::operator = ( const JoypadKeyMap& jkm )
{
	memcpy( keys, jkm.keys, JS_BUTTON_COUNT * sizeof(int) );
	return *this;
}


void JoypadKeyMap::SetKey( int key, JOYSTICK_BUTTON button )
{
	keys[ button ] = key;
}


JOYSTICK_BUTTON JoypadKeyMap::KeyCodeToButton( int key ) const
{
	if ( key == -1 )
		return JS_BUTTON_INVALID;

	for ( int i=0; i<JS_BUTTON_COUNT; ++i )
	{
		if ( keys[ i ] == key )
			return (JOYSTICK_BUTTON)i;
	}

	return JS_BUTTON_INVALID;
}


int JoypadKeyMap::ButtonToKeyCode( JOYSTICK_BUTTON button ) const
{
	return keys[ button ];
}
