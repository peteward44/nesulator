
#include "../main.h"
#include "../options.h"
#include "controldialog.h"
#include "../mainboard.h"


class ControlDialog : public wxDialog
{
private:
	DECLARE_EVENT_TABLE();

	int joystick;

	wxButton* guiButtons[ JS_BUTTON_COUNT ];

	void LabelButton( JOYSTICK_BUTTON button, int key );
	std::string GetButtonName( JOYSTICK_BUTTON button );
	std::string GetKeyName( int key );

public:
	ControlDialog( wxWindow* parent, int joystick );

	void OnOKButton( wxCommandEvent& ev );
	void OnControlButton( wxCommandEvent& ev );
};


class PressKeyDialog : public wxDialog
{
private:
	DECLARE_EVENT_TABLE();
	int key;

public:

	int GetKey() { return key; }

	PressKeyDialog( wxWindow* parent )
		: wxDialog( parent, -1, "Press a key", wxDefaultPosition, wxSize( 120, 60 ), wxSTAY_ON_TOP | wxCAPTION )
	{ key = -1; }

	void OnKeyUp(wxKeyEvent& ev)
	{
		key = ev.GetKeyCode();
		this->EndModal( 0 );
	}
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


enum CONFIGCONTROLSMENU
{
	CCMENU_CLOSE,
	CCMENU_RADIOBOX_0,
	CCMENU_RADIOBOX_1,
	CCMENU_CONFIGKEYS_0,
	CCMENU_CONFIGKEYS_1
};


BEGIN_EVENT_TABLE(PressKeyDialog, wxDialog)
	EVT_KEY_UP(PressKeyDialog::OnKeyUp)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(ControlDialog, wxDialog)
	EVT_BUTTON( 0xFF,		ControlDialog::OnOKButton)
	EVT_BUTTON( JS_A,		ControlDialog::OnControlButton)
	EVT_BUTTON( JS_B,		ControlDialog::OnControlButton)
	EVT_BUTTON( JS_START,	ControlDialog::OnControlButton)
	EVT_BUTTON( JS_SELECT,	ControlDialog::OnControlButton)
	EVT_BUTTON( JS_UP,		ControlDialog::OnControlButton)
	EVT_BUTTON( JS_DOWN,	ControlDialog::OnControlButton)
	EVT_BUTTON( JS_LEFT,	ControlDialog::OnControlButton)
	EVT_BUTTON( JS_RIGHT,	ControlDialog::OnControlButton)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(ConfigControlsDialog, wxDialog)
	EVT_BUTTON( CCMENU_CLOSE,		ConfigControlsDialog::OnCloseButton )
	EVT_RADIOBOX( CCMENU_RADIOBOX_0,	ConfigControlsDialog::OnRadioBoxChange )
	EVT_RADIOBOX( CCMENU_RADIOBOX_1,	ConfigControlsDialog::OnRadioBoxChange )
	EVT_BUTTON( CCMENU_CONFIGKEYS_0,	ConfigControlsDialog::OnConfigKeysButton )
	EVT_BUTTON( CCMENU_CONFIGKEYS_1,	ConfigControlsDialog::OnConfigKeysButton )
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ConfigControlsDialog::ConfigControlsDialog( wxWindow* parent )
	: wxDialog( parent, -1, "Configure Controls", wxDefaultPosition, wxSize( 335, 200 ), wxCAPTION | wxSYSTEM_MENU )
{
	const int buttonWidth = 70;
	const char* deviceOptions[] = { "No device connected", "Joypad", "Zapper" };

	for ( int i=0; i<2; ++i )
	{
		radioBoxes[i] = new wxRadioBox( this, CCMENU_RADIOBOX_0 + i, ( boost::format( "Controller %1%" ) % ( i + 1 ) ).str().c_str(), wxPoint( 10 + ( 160 * i ), 10 ), wxSize( 150, 85 ),
			wxArrayString( 3, deviceOptions ), 0, wxRA_SPECIFY_ROWS );
		configButtons[i] = new wxButton( this, CCMENU_CONFIGKEYS_0 + i, "Configure Keys", wxPoint( 40 + ( 160 * i ), 105 ), wxSize( 100, 20 ) );

		radioBoxes[i]->SetSelection( g_options->GetDeviceType( i ) );

		EnableConfigButton( i );
	}

	wxButton *btnClose = new wxButton( this, CCMENU_CLOSE, "Close", wxPoint( 140, 140 ), wxSize( 50, 20 ) );
}


void ConfigControlsDialog::EnableConfigButton( int controller )
{
	switch ( radioBoxes[ controller ]->GetSelection() )
	{
	case INPUT_NONE: // no device connected
	case INPUT_ZAPPER: // zapper
		configButtons[ controller ]->Enable( false );
		break;
	case INPUT_JOYPAD: // joypad
		configButtons[ controller ]->Enable( true );
		break;
	}
}


void ConfigControlsDialog::OnCloseButton( wxCommandEvent& ev )
{
	for ( int controller=0; controller<2; ++controller )
	{
		g_options->SetDeviceType( controller, (INPUT_DEVICE)radioBoxes[ controller ]->GetSelection() );
	}

	g_mainboard->GetInputDeviceBus()->SyncWithOptions();

	this->Close();
}


void ConfigControlsDialog::OnConfigKeysButton( wxCommandEvent& ev )
{
	ControlDialog* d = new ControlDialog( this, ev.GetId() - CCMENU_CONFIGKEYS_0 );
	d->ShowModal();
}


void ConfigControlsDialog::OnRadioBoxChange( wxCommandEvent& ev )
{
	EnableConfigButton( ev.GetId() - CCMENU_RADIOBOX_0 );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ControlDialog::ControlDialog( wxWindow* parent, int joystick )
	: wxDialog( parent, -1, "Configure Joypad", wxDefaultPosition, wxSize( 380, 150 ), wxCAPTION | wxSYSTEM_MENU ), joystick( joystick )
{
	const int buttonWidth = 70;

	guiButtons[ JS_START ] = new wxButton( this, JS_START, wxEmptyString, wxPoint( 10, 55 ), wxSize( buttonWidth, 20 ) );
	guiButtons[ JS_SELECT ] = new wxButton( this, JS_SELECT, wxEmptyString, wxPoint( 10 + buttonWidth + 10, 55 ), wxSize( buttonWidth, 20 ) );

	guiButtons[ JS_B ] = new wxButton( this, JS_B, wxEmptyString, wxPoint( 10, 30 ), wxSize( buttonWidth, 20 ) );
	guiButtons[ JS_A ] = new wxButton( this, JS_A, wxEmptyString, wxPoint( 10 + buttonWidth + 10, 30 ), wxSize( buttonWidth, 20 ) );

	guiButtons[ JS_LEFT ] = new wxButton( this, JS_LEFT, wxEmptyString, wxPoint( buttonWidth + 75 + 40, 50 ), wxSize( buttonWidth, 20 ) );
	guiButtons[ JS_RIGHT ] = new wxButton( this, JS_RIGHT, wxEmptyString, wxPoint( buttonWidth + 185 + 40, 50 ), wxSize( buttonWidth, 20 ) );
	guiButtons[ JS_DOWN ] = new wxButton( this, JS_DOWN, wxEmptyString, wxPoint( buttonWidth + 130 + 40, 70 ), wxSize( buttonWidth, 20 ) );
	guiButtons[ JS_UP ] = new wxButton( this, JS_UP, wxEmptyString, wxPoint( buttonWidth + 130 + 40, 30 ), wxSize( buttonWidth, 20 ) );

	for ( int i=0; i<JS_BUTTON_COUNT; ++i )
	{
		LabelButton( (JOYSTICK_BUTTON)i, g_options->GetJoypadKeyMap( joystick ).ButtonToKeyCode( (JOYSTICK_BUTTON)i ) );
	}

	new wxButton( this, 0xFF, "Close", wxPoint( 165, 90 ), wxSize( 50, 20 ) );
}


std::string ControlDialog::GetButtonName( JOYSTICK_BUTTON button )
{
	std::string buttonname;
	switch ( button )
	{
	case JS_A:
		buttonname = "A";
		break;
	case JS_B:
		buttonname = "B";
		break;
	case JS_SELECT:
		buttonname = "Select";
		break;
	case JS_START:
		buttonname = "Start";
		break;
	case JS_UP:
		buttonname = "Up";
		break;
	case JS_DOWN:
		buttonname = "Down";
		break;
	case JS_LEFT:
		buttonname = "Left";
		break;
	case JS_RIGHT:
		buttonname = "Right";
		break;
	}
	return buttonname;
}


std::string ControlDialog::GetKeyName( int key )
{
	std::string keyname;

	switch ( key )
	{
	case -1:
		keyname = "";
		break;
	case WXK_BACK:
		keyname = "BCK";
		break;
	case WXK_TAB:
		keyname = "TAB";
		break;
	case WXK_RETURN:
		keyname = "RTN";
		break;
	case WXK_ESCAPE:
		keyname = "ESC";
		break;
	case WXK_SPACE:
		keyname = "SPC";
		break;
	case WXK_DELETE:
		keyname = "DEL";
		break;
	case WXK_START:
		keyname = "START";
		break;
	case WXK_LBUTTON:
		keyname = "LBUTTON";
		break;
	case WXK_RBUTTON:
		keyname = "RBUTTON";
		break;
	case WXK_CANCEL:
		keyname = "CANCEL";
		break;
	case WXK_MBUTTON:
		keyname = "MBUTTON";
		break;
	case WXK_CLEAR:
		keyname = "CLEAR";
		break;
	case WXK_SHIFT:
		keyname = "SHIFT";
		break;
	case WXK_ALT:
		keyname = "ALT";
		break;
	case WXK_CONTROL:
		keyname = "CTRL";
		break;
	case WXK_MENU:
		keyname = "MENU";
		break;
	case WXK_PAUSE:
		keyname = "PAUSE";
		break;
	case WXK_CAPITAL:
		keyname = "CAP";
		break;
	case WXK_END:
		keyname = "END";
		break;
	case WXK_HOME:
		keyname = "HOME";
		break;
	case WXK_LEFT:
		keyname = "LEFT";
		break;
	case WXK_UP:
		keyname = "UP";
		break;
	case WXK_RIGHT:
		keyname = "RIGHT";
		break;
	case WXK_DOWN:
		keyname = "DOWN";
		break;
	case WXK_SELECT:
		keyname = "SELECT";
		break;
	case WXK_PRINT:
		keyname = "PRINT";
		break;
	case WXK_EXECUTE:
		keyname = "EXEC";
		break;
	case WXK_SNAPSHOT:
		keyname = "PRNTSCRN";
		break;
	case WXK_INSERT:
		keyname = "INS";
		break;
	case WXK_HELP:
		keyname = "HELP";
		break;
	case WXK_NUMPAD0:
		keyname = "NUM0";
		break;
	case WXK_NUMPAD1:
		keyname = "NUM1";
		break;
	case WXK_NUMPAD2:
		keyname = "NUM2";
		break;
	case WXK_NUMPAD3:
		keyname = "NUM3";
		break;
	case WXK_NUMPAD4:
		keyname = "NUM4";
		break;
	case WXK_NUMPAD5:
		keyname = "NUM5";
		break;
	case WXK_NUMPAD6:
		keyname = "NUM6";
		break;
	case WXK_NUMPAD7:
		keyname = "NUM7";
		break;
	case WXK_NUMPAD8:
		keyname = "NUM8";
		break;
	case WXK_NUMPAD9:
		keyname = "NUM9";
		break;
	case WXK_MULTIPLY:
		keyname = "MULT";
		break;
	case WXK_ADD:
		keyname = "ADD";
		break;
	case WXK_SEPARATOR:
		keyname = "SEP";
		break;
	case WXK_SUBTRACT:
		keyname = "SUB";
		break;
	case WXK_DECIMAL:
		keyname = "DEC";
		break;
	case WXK_DIVIDE:
		keyname = "DIV";
		break;
	case WXK_F1:
		keyname = "F1";
		break;
	case WXK_F2:
		keyname = "F2";
		break;
	case WXK_F3:
		keyname = "F3";
		break;
	case WXK_F4:
		keyname = "F4";
		break;
	case WXK_F5:
		keyname = "F5";
		break;
	case WXK_F6:
		keyname = "F6";
		break;
	case WXK_F7:
		keyname = "F7";
		break;
	case WXK_F8:
		keyname = "F8";
		break;
	case WXK_F9:
		keyname = "F9";
		break;
	case WXK_F10:
		keyname = "F10";
		break;
	case WXK_F11:
		keyname = "F11";
		break;
	case WXK_F12:
		keyname = "F12";
		break;
	case WXK_F13:
		keyname = "F13";
		break;
	case WXK_F14:
		keyname = "F14";
		break;
	case WXK_F15:
		keyname = "F15";
		break;
	case WXK_F16:
		keyname = "F16";
		break;
	case WXK_F17:
		keyname = "F17";
		break;
	case WXK_F18:
		keyname = "F18";
		break;
	case WXK_F19:
		keyname = "F19";
		break;
	case WXK_F20:
		keyname = "F20";
		break;
	case WXK_F21:
		keyname = "F21";
		break;
	case WXK_F22:
		keyname = "F22";
		break;
	case WXK_F23:
		keyname = "F23";
		break;
	case WXK_F24:
		keyname = "F24";
		break;
	case WXK_NUMLOCK:
		keyname = "NUMLOCK";
		break;
	case WXK_SCROLL:
		keyname = "SCROLL";
		break;
	case WXK_PAGEUP:
		keyname = "PAGEUP";
		break;
	case WXK_PAGEDOWN:
		keyname = "PAGEDOWN";
		break;

	case WXK_NUMPAD_SPACE:
		keyname = "NUM SPACE";
		break;
	case WXK_NUMPAD_TAB:
		keyname = "NUM TAB";
		break;
	case WXK_NUMPAD_ENTER:
		keyname = "NUM ENTER";
		break;
	case WXK_NUMPAD_F1:
		keyname = "NUM F1";
		break;
	case WXK_NUMPAD_F2:
		keyname = "NUM F2";
		break;
	case WXK_NUMPAD_F3:
		keyname = "NUM F3";
		break;
	case WXK_NUMPAD_F4:
		keyname = "NUM F4";
		break;
	case WXK_NUMPAD_HOME:
		keyname = "NUM HOME";
		break;
	case WXK_NUMPAD_LEFT:
		keyname = "NUM LEFT";
		break;
	case WXK_NUMPAD_UP:
		keyname = "NUM UP";
		break;
	case WXK_NUMPAD_RIGHT:
		keyname = "NUM RIGHT";
		break;
	case WXK_NUMPAD_DOWN:
		keyname = "NUM DOWN";
		break;
	case WXK_NUMPAD_PAGEUP:
		keyname = "NUM PAGEUP";
		break;
	case WXK_NUMPAD_PAGEDOWN:
		keyname = "NUM PAGEDOWN";
		break;
	case WXK_NUMPAD_END:
		keyname = "NUM END";
		break;
	case WXK_NUMPAD_BEGIN:
		keyname = "NUM BEGIN";
		break;
	case WXK_NUMPAD_INSERT:
		keyname = "NUM INS";
		break;
	case WXK_NUMPAD_DELETE:
		keyname = "NUM DEL";
		break;
	case WXK_NUMPAD_EQUAL:
		keyname = "NUM EQUAL";
		break;
	case WXK_NUMPAD_MULTIPLY:
		keyname = "NUM MULT";
		break;
	case WXK_NUMPAD_ADD:
		keyname = "NUM ADD";
		break;
	case WXK_NUMPAD_SEPARATOR:
		keyname = "NUM SEP";
		break;
	case WXK_NUMPAD_SUBTRACT:
		keyname = "NUM SUB";
		break;
	case WXK_NUMPAD_DECIMAL:
		keyname = "NUM DEC";
		break;
	case WXK_NUMPAD_DIVIDE:
		keyname = "NUM DIV";
		break;

	case WXK_WINDOWS_LEFT:
		keyname = "WIN LEFT";
		break;
	case WXK_WINDOWS_RIGHT:
		keyname = "WIN RIGHT";
		break;
	case WXK_WINDOWS_MENU:
		keyname = "WIN MENU";
		break;
	case WXK_COMMAND:
		keyname = "COMMAND";
		break;

	case WXK_SPECIAL1:
		keyname = "SPEC1";
		break;
	case WXK_SPECIAL2:
		keyname = "SPEC2";
		break;
	case WXK_SPECIAL3:
		keyname = "SPEC3";
		break;
	case WXK_SPECIAL4:
		keyname = "SPEC4";
		break;
	case WXK_SPECIAL5:
		keyname = "SPEC5";
		break;
	case WXK_SPECIAL6:
		keyname = "SPEC6";
		break;
	case WXK_SPECIAL7:
		keyname = "SPEC7";
		break;
	case WXK_SPECIAL8:
		keyname = "SPEC8";
		break;
	case WXK_SPECIAL9:
		keyname = "SPEC9";
		break;
	case WXK_SPECIAL10:
		keyname = "SPEC10";
		break;
	case WXK_SPECIAL11:
		keyname = "SPEC11";
		break;
	case WXK_SPECIAL12:
		keyname = "SPEC12";
		break;
	case WXK_SPECIAL13:
		keyname = "SPEC13";
		break;
	case WXK_SPECIAL14:
		keyname = "SPEC14";
		break;
	case WXK_SPECIAL15:
		keyname = "SPEC15";
		break;
	case WXK_SPECIAL16:
		keyname = "SPEC16";
		break;
	case WXK_SPECIAL17:
		keyname = "SPEC17";
		break;
	case WXK_SPECIAL18:
		keyname = "SPEC18";
		break;
	case WXK_SPECIAL19:
		keyname = "SPEC19";
		break;
	case WXK_SPECIAL20:
		keyname = "SPEC20";
		break;
	default:
		keyname = ( boost::format( "%1%" ) % (char)key ).str();
		break;
	}
	return keyname;
}


void ControlDialog::LabelButton( JOYSTICK_BUTTON button, int key )
{
	guiButtons[ button ]->SetLabel( ( boost::format( "%1% - %2%" ) % GetButtonName( button ) % GetKeyName( key ) ).str().c_str() );
}


void ControlDialog::OnOKButton(wxCommandEvent& ev)
{
	EndModal( wxID_OK );
}


void ControlDialog::OnControlButton( wxCommandEvent& ev )
{
	PressKeyDialog* pressKeyDialog = new PressKeyDialog( this );
	pressKeyDialog->ShowModal();

	JOYSTICK_BUTTON button = (JOYSTICK_BUTTON)ev.GetId();

	g_options->GetJoypadKeyMap( joystick ).SetKey( pressKeyDialog->GetKey(), button );

	LabelButton( button, pressKeyDialog->GetKey() );
}

