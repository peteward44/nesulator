

#include "mainboard.h"
#include "options.h"
#include "gui/app.h"


InputDeviceBus::InputDeviceBus()
{
	for ( int i=0; i<2; ++i )
		ConnectDevice( i, CreateInputDevice( INPUT_NONE ) );

	g_options->AttachToSyncEvent( boost::bind( &InputDeviceBus::SyncWithOptions, this ) );
}


void InputDeviceBus::SyncWithOptions()
{
	for ( int i=0; i<2; ++i )
	{
		ConnectDevice( i, CreateInputDevice( g_options->GetDeviceType( i ) ) );
	}
}


void InputDeviceBus::WriteToRegister( UInt16_t offset, Byte_t data )
{
	switch ( offset )
	{
	case 0x4016:
		devices[ 0 ]->WriteToRegister( offset, data );
		break;
	case 0x4017:
		devices[ 1 ]->WriteToRegister( offset, data );
		break;
	}
}


Byte_t InputDeviceBus::ReadFromRegister( UInt16_t offset )
{
	Byte_t ret = 0;

	switch ( offset )
	{
	case 0x4016:
		ret = devices[ 0 ]->ReadFromRegister( offset );
		break;
	case 0x4017:
		ret = devices[ 1 ]->ReadFromRegister( offset );
		break;
	}

	return ret;
}


///////////////////////////////////////


void InputDeviceBus::ConnectDevice( int port, InputDevicePtr_t device )
{
	assert( port >= 0 && port < 2 );
	devices[ port ] = device;
	devices[ port ]->OnConnection( port );
}


void InputDeviceBus::DisconnectDevice( int port )
{
	assert( port >= 0 && port < 2 );
	devices[ port ]->OnDisconnection( port );
	devices[ port ] = CreateInputDevice( INPUT_NONE );
}


InputDevicePtr_t InputDeviceBus::GetDevice( int port )
{
	assert( port >= 0 && port < 2 );
	return devices[ port ];
}


///////////////////////////////////////


void InputDeviceBus::OnKeyUp( int keyCode )
{
	for ( int i=0; i<2; ++i )
	{
		devices[i]->OnKeyUp( keyCode );
	}

	// debugging methods
	switch ( keyCode )
	{
	//case 'P':
	//	g_nesMainboard->Pause( !g_nesMainboard->IsPaused() );
	//	break;
	//case 'O':
	//	g_nesMainboard->StepFrame( !g_nesMainboard->IsStepFrame() );
	//	break;
	case WXK_HOME:
		if ( g_activeMainboard != NULL )
			g_activeMainboard->TakeScreenshot();
		break;
	//case 'I':
	//	g_nesMainboard->GetInputDeviceBus()->OnMouseLeftDown( 1, 1 );
	//	g_nesMainboard->StepFrame( !g_nesMainboard->IsStepFrame() );
	//	break;
	//case 'U':
	//	g_nesMainboard->GetInputDeviceBus()->OnMouseLeftUp( 1, 1 );
	//	g_nesMainboard->StepFrame( !g_nesMainboard->IsStepFrame() );
	//	break;
	}
}


void InputDeviceBus::OnKeyDown( int keyCode )
{
	for ( int i=0; i<2; ++i )
	{
		devices[i]->OnKeyDown( keyCode );
	}
}


void InputDeviceBus::OnMouseLeftDown( int x, int y )
{
	for ( int i=0; i<2; ++i )
	{
		devices[i]->OnMouseLeftDown( x, y );
	}
}


void InputDeviceBus::OnMouseLeftUp( int x, int y )
{
	for ( int i=0; i<2; ++i )
	{
		devices[i]->OnMouseLeftUp( x, y );
	}
}


///////////////////////////////////////


void InputDeviceBus::SaveState( std::ostream& ostr )
{
	for ( int i=0; i<2; ++i )
	{
		WriteType< boost::uint8_t >( ostr, devices[i]->GetDeviceType() );
		devices[i]->SaveState( ostr );
	}
}


void InputDeviceBus::LoadState( std::istream& istr )
{
	for ( int i=0; i<2; ++i )
	{
		ConnectDevice( i, CreateInputDevice( (INPUT_DEVICE)ReadType< boost::uint8_t >( istr ) ) );
		devices[i]->LoadState( istr );
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////


Joypad::Joypad()
{
	nextJoystickButton = 0;
	joystickHigh = joystickLow = false;
	currentPort = -1;

	for ( int i=0; i<JS_BUTTON_COUNT; ++i )
	  buttons[ i ] = false;
}


void Joypad::OnConnection( int port )
{
	currentPort = port;
}


void Joypad::OnDisconnection( int port )
{
	currentPort = -1;
}


void Joypad::WriteToRegister( UInt16_t offset, Byte_t data )
{
	if ( (data & 0x1) == 1) joystickHigh = true;
	if ( (data & 0x1) == 0) joystickLow = true;
	if ((joystickHigh == true) && (joystickLow == true))
	{
		nextJoystickButton = 0;
		joystickHigh = false;
		joystickLow = false;
	}
}


Byte_t Joypad::ReadFromRegister( UInt16_t offset )
{
	Byte_t ret = buttons[ nextJoystickButton & 0x7 ] ? 0x1 : 0x0;
	nextJoystickButton++;
	return ret;
}


void Joypad::SaveState( std::ostream& ostr )
{
	WriteType< boost::uint32_t >( ostr, nextJoystickButton );
	WriteType< boost::uint8_t >( ostr, joystickHigh ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, joystickLow ? 1 : 0 );
}


void Joypad::LoadState( std::istream& istr )
{
	nextJoystickButton = ReadType< boost::uint32_t >( istr );
	joystickHigh = ReadType< boost::uint8_t >( istr ) > 0;
	joystickLow = ReadType< boost::uint8_t >( istr ) > 0;
}


void Joypad::OnKeyUp( int keyCode )
{
	assert( currentPort >= 0 );
	JOYSTICK_BUTTON button = g_options->GetJoypadKeyMap( currentPort ).KeyCodeToButton( keyCode );
	if ( button != JS_BUTTON_INVALID )
		buttons[ button ] = false;
}


void Joypad::OnKeyDown( int keyCode )
{
	assert( currentPort >= 0 );
	JOYSTICK_BUTTON button = g_options->GetJoypadKeyMap( currentPort ).KeyCodeToButton( keyCode );
	if ( button != JS_BUTTON_INVALID )
		buttons[ button ] = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////


Zapper::Zapper()
{
	triggerHeld = false;
}


void Zapper::WriteToRegister( UInt16_t offset, Byte_t data )
{
}


Byte_t Zapper::ReadFromRegister( UInt16_t offset )
{
	Byte_t ret = 0;

	if ( triggerHeld )
		ret |= 0x10;
	
	if ( !IsSpriteInCrossHairs() )
		ret |= 0x08;

	return ret;
}


void Zapper::SaveState( std::ostream& ostr )
{
}


void Zapper::LoadState( std::istream& istr )
{
}


void Zapper::OnMouseLeftDown( int x, int y )
{
	triggerHeld = true;
}


void Zapper::OnMouseLeftUp( int x, int y )
{
	triggerHeld = false;
}


bool Zapper::IsSpriteInCrossHairs() const
{
	int x, y;
	wxPoint clientCoords = wxGetApp().GetMainFrame()->ScreenToClient( wxGetMousePosition() );
	wxGetApp().GetMainFrame()->GetCanvas()->CoordinatesToCanvas( clientCoords.x, clientCoords.y, x, y );

	if ( x < 0 || x >= 256 || y < 0 || y >= 240 || !g_nesMainboard->GetPPU()->IsRenderingEnabled() )
		return false;
	else
	{
	// check renderbuffer last frame. see if there are white pixels in the region of the hit
		int whiteCount = 0;
		const Byte_t* buffer = g_nesMainboard->GetNesRenderBuffer()->GetPaletteBuffer( );

		for ( int yy=y-8; yy<y+8; ++yy )
		{
			if ( yy < 0 ) 
				yy = 0;
			
			if ( yy >= 240 )
				break;

			for ( int xx=x-8; xx<x+8; ++xx )
			{
				if ( xx < 0 )
					xx = 0;

				if ( xx >= 256 )
					break;

				if ( IsWhitePixel( buffer[ yy*256 + xx ] & 0x3F ) )
					whiteCount++;
			}
		}

		return whiteCount > 64;
	}
}


bool Zapper::IsWhitePixel( Byte_t paletteIndex ) const
{
	// NOTE: table below was nicked from Nintendulator
	static const bool	IsWhite[0x40] =
	{
		true,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,
		true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	false,	false,	false,
		true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	false,	false,
		true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	true,	false,	false
	};

	return IsWhite[ paletteIndex ];
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////


InputDevicePtr_t CreateInputDevice( INPUT_DEVICE device )
{
	switch ( device )
	{
	case INPUT_JOYPAD:
		return InputDevicePtr_t( new Joypad() );
	case INPUT_ZAPPER:
		return InputDevicePtr_t( new Zapper() );
	case INPUT_NONE:
		return InputDevicePtr_t( new NullDevice() );
	default:
		Log::Write( LOG_ERROR, ( boost::wformat( L"Invalid input device attempted to be created: %1%" ) % device ).str() );
		throw std::runtime_error( "Invalid input device" );
	}
}

