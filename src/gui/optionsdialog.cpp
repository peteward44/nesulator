

#include "../main.h"
#include "../options.h"
#include "optionsdialog.h"
#include "../mainboard.h"
#include "app.h"

class ControlDialog : public wxDialog
{
private:
	DECLARE_EVENT_TABLE();

	int joystick;

	wxButton* guiButtons[ JS_BUTTON_COUNT ];

	void LabelButton( JOYSTICK_BUTTON button, int key );
	std::wstring GetButtonName( JOYSTICK_BUTTON button );
	std::wstring GetKeyName( int key );

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


enum OPTIONSMENU
{
	OPTIONSMENU_OK,
	OPTIONSMENU_CANCEL,
	OPTIONSMENU_RADIOBOX_0,
	OPTIONSMENU_RADIOBOX_1,
	OPTIONSMENU_CONFIGKEYS_0,
	OPTIONSMENU_CONFIGKEYS_1,
	OPTIONSMENU_SAVESTATEBROWSE,
	OPTIONSMENU_SCREENSHOTBROWSE
};



BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
	EVT_BUTTON( OPTIONSMENU_OK,		OptionsDialog::OnOkButton )
	EVT_BUTTON( OPTIONSMENU_CANCEL,		OptionsDialog::OnCancelButton )
	EVT_RADIOBOX( OPTIONSMENU_RADIOBOX_0,	OptionsDialog::OnRadioBoxChange )
	EVT_RADIOBOX( OPTIONSMENU_RADIOBOX_1,	OptionsDialog::OnRadioBoxChange )
	EVT_BUTTON( OPTIONSMENU_CONFIGKEYS_0,	OptionsDialog::OnConfigKeysButton )
	EVT_BUTTON( OPTIONSMENU_CONFIGKEYS_1,	OptionsDialog::OnConfigKeysButton )
	EVT_BUTTON( OPTIONSMENU_SAVESTATEBROWSE,	OptionsDialog::OnDirectoryBrowseButton )
	EVT_BUTTON( OPTIONSMENU_SCREENSHOTBROWSE,	OptionsDialog::OnDirectoryBrowseButton )
END_EVENT_TABLE()



void OptionsDialog::EnableConfigButton( int controller )
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


OptionsDialog::OptionsDialog( wxWindow* parent )
	: wxDialog( parent, -1, "Options", wxDefaultPosition, wxSize( 595, 320 ) )
{
	// video
	wxStaticBox* videoBox = new wxStaticBox( this, -1, L"Video", wxPoint( 10, 10 ), wxSize( 260, 135 ) );

	cbAllowSprites = new wxCheckBox( videoBox, -1, L"Allow more than 8 sprites per scanline", wxPoint( 10, 25 ) );
	cbAllowSprites->SetValue( g_options->AllowMoreThanEightSprites );

	const wchar_t* filteringOptions[] = { L"No filtering ", L"NTSC filtering" };
	mFilteringRadioBox = new wxRadioBox( videoBox, -1, L"Filtering method", wxPoint( 10, 55 ), wxSize( 240, 65 ),
		wxArrayString( 2, filteringOptions ), 0, wxRA_SPECIFY_ROWS );
	mFilteringRadioBox->SetSelection( g_options->UseNtscFiltering ? 1 : 0 );

	// sound
	wxStaticBox* soundBox = new wxStaticBox( this, -1, L"Sound", wxPoint( 10, 150 ), wxSize( 260, 100 ) );

	cbEnableSound = new wxCheckBox( soundBox, -1, L"Enable Sound", wxPoint( 10, 25 ) );
	cbEnableSound->SetValue( g_options->SoundEnabled );
	//new wxStaticText( soundBox, -1, "Volume:", wxPoint( 10, 55 ) );
	slVolume = new wxSlider( soundBox, -1, g_options->SoundVolume, SOUND_VOLUME_MIN, SOUND_VOLUME_MAX, wxPoint( 60, 50 ), wxSize( 160, 40 ) );
	slVolume->Hide( );

	// controls
	wxStaticBox* controlsBox = new wxStaticBox( this, -1, L"Controls", wxPoint( 280, 10 ), wxSize( 300, 135 ) );
	const int buttonWidth = 70;
	const wchar_t* deviceOptions[] = { L"No device connected", L"Joypad", L"Zapper" };

	for ( int i=0; i<2; ++i )
	{
		radioBoxes[i] = new wxRadioBox( controlsBox, OPTIONSMENU_RADIOBOX_0 + i, ( boost::wformat( L"Controller %1%" ) % ( i + 1 ) ).str().c_str(),
			wxPoint( 10 + ( 145 * i ), 15 ), wxSize( 135, 85 ), wxArrayString( 3, deviceOptions ), 0, wxRA_SPECIFY_ROWS );

		configButtons[i] = new wxButton( controlsBox, OPTIONSMENU_CONFIGKEYS_0 + i, L"Configure Keys", wxPoint( 40 + ( 145 * i ), 105 ), wxSize( 100, 20 ) );

		radioBoxes[i]->SetSelection( g_options->GetDeviceType( i ) );

		EnableConfigButton( i );
	}

	// directories (savestates, screenshots)
	wxStaticBox* dirBox = new wxStaticBox( this, -1, L"Directories", wxPoint( 280, 150 ), wxSize( 300, 100 ) );

	new wxStaticText( dirBox, -1, L"Save States:", wxPoint( 10, 25 ) );
	tcSaveState = new wxTextCtrl( dirBox, -1, L"", wxPoint( 80, 20 ), wxSize( 145, 20 ) );
	tcSaveState->SetValue( g_options->SaveStatesDirectory.c_str() );
	new wxButton( dirBox, OPTIONSMENU_SAVESTATEBROWSE, L"Browse", wxPoint( 230, 20 ), wxSize( 60, 20 ) );

	new wxStaticText( dirBox, -1, L"Screenshots:", wxPoint( 10, 55 ) );
	tcScreenshot = new wxTextCtrl( dirBox, -1, L"", wxPoint( 80, 50 ), wxSize( 145, 20 ) );
	tcScreenshot->SetValue( g_options->ScreenshotsDirectory.c_str() );
	new wxButton( dirBox, OPTIONSMENU_SCREENSHOTBROWSE, L"Browse", wxPoint( 230, 50 ), wxSize( 60, 20 ) );

	new wxButton( this, OPTIONSMENU_OK, L"OK", wxPoint( 450, 260 ), wxSize( 60, 25 ) );
	new wxButton( this, OPTIONSMENU_CANCEL, L"Cancel", wxPoint( 520, 260 ), wxSize( 60, 25 ) );
}


void OptionsDialog::OnOkButton( wxCommandEvent& ev )
{
	g_options->AllowMoreThanEightSprites = cbAllowSprites->IsChecked();
	g_options->UseNtscFiltering = mFilteringRadioBox->GetSelection() > 0;

	g_options->SoundEnabled = cbEnableSound->IsChecked();
	g_options->SoundVolume = slVolume->GetValue();

	g_options->SaveStatesDirectory = tcSaveState->GetValue().c_str();
	g_options->ScreenshotsDirectory = tcScreenshot->GetValue().c_str();

	for ( int controller=0; controller<2; ++controller )
	{
		g_options->SetDeviceType( controller, (INPUT_DEVICE)radioBoxes[ controller ]->GetSelection() );
	}

	g_options->SaveToFile( wxGetApp().GetConfigFilename() );

	g_options->InvokeSyncEvent();

	this->Close();
}


void OptionsDialog::OnCancelButton( wxCommandEvent& ev )
{
	this->Close();
}


void OptionsDialog::OnRadioBoxChange( wxCommandEvent& ev )
{
	EnableConfigButton( ev.GetId() - OPTIONSMENU_RADIOBOX_0 );
}


void OptionsDialog::OnConfigKeysButton( wxCommandEvent& ev )
{
	ControlDialog* d = new ControlDialog( this, ev.GetId() - OPTIONSMENU_CONFIGKEYS_0 );
	d->ShowModal();
}


void OptionsDialog::OnDirectoryBrowseButton( wxCommandEvent& ev )
{
	wxDirDialog* dirDialog = new wxDirDialog( this, L"Select directory", L"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );
	
	if ( dirDialog->ShowModal() == wxID_OK )
	{
		switch ( ev.GetId() )
		{
		case OPTIONSMENU_SAVESTATEBROWSE:
			tcSaveState->SetValue( dirDialog->GetPath() );
			break;
		case OPTIONSMENU_SCREENSHOTBROWSE:
			tcScreenshot->SetValue( dirDialog->GetPath() );
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

	new wxButton( this, 0xFF, L"Close", wxPoint( 165, 90 ), wxSize( 50, 20 ) );
}


std::wstring ControlDialog::GetButtonName( JOYSTICK_BUTTON button )
{
	std::wstring buttonname;
	switch ( button )
	{
	case JS_A:
		buttonname = L"A";
		break;
	case JS_B:
		buttonname = L"B";
		break;
	case JS_SELECT:
		buttonname = L"Select";
		break;
	case JS_START:
		buttonname = L"Start";
		break;
	case JS_UP:
		buttonname = L"Up";
		break;
	case JS_DOWN:
		buttonname = L"Down";
		break;
	case JS_LEFT:
		buttonname = L"Left";
		break;
	case JS_RIGHT:
		buttonname = L"Right";
		break;
	}
	return buttonname;
}


std::wstring ControlDialog::GetKeyName( int key )
{
	std::wstring keyname;

	switch ( key )
	{
	case -1:
		keyname = L"";
		break;
	case WXK_BACK:
		keyname = L"BCK";
		break;
	case WXK_TAB:
		keyname = L"TAB";
		break;
	case WXK_RETURN:
		keyname = L"RTN";
		break;
	case WXK_ESCAPE:
		keyname = L"ESC";
		break;
	case WXK_SPACE:
		keyname = L"SPC";
		break;
	case WXK_DELETE:
		keyname = L"DEL";
		break;
	case WXK_START:
		keyname = L"START";
		break;
	case WXK_LBUTTON:
		keyname = L"LBUTTON";
		break;
	case WXK_RBUTTON:
		keyname = L"RBUTTON";
		break;
	case WXK_CANCEL:
		keyname = L"CANCEL";
		break;
	case WXK_MBUTTON:
		keyname = L"MBUTTON";
		break;
	case WXK_CLEAR:
		keyname = L"CLEAR";
		break;
	case WXK_SHIFT:
		keyname = L"SHIFT";
		break;
	case WXK_ALT:
		keyname = L"ALT";
		break;
	case WXK_CONTROL:
		keyname = L"CTRL";
		break;
	case WXK_MENU:
		keyname = L"MENU";
		break;
	case WXK_PAUSE:
		keyname = L"PAUSE";
		break;
	case WXK_CAPITAL:
		keyname = L"CAP";
		break;
	case WXK_END:
		keyname = L"END";
		break;
	case WXK_HOME:
		keyname = L"HOME";
		break;
	case WXK_LEFT:
		keyname = L"LEFT";
		break;
	case WXK_UP:
		keyname = L"UP";
		break;
	case WXK_RIGHT:
		keyname = L"RIGHT";
		break;
	case WXK_DOWN:
		keyname = L"DOWN";
		break;
	case WXK_SELECT:
		keyname = L"SELECT";
		break;
	case WXK_PRINT:
		keyname = L"PRINT";
		break;
	case WXK_EXECUTE:
		keyname = L"EXEC";
		break;
	case WXK_SNAPSHOT:
		keyname = L"PRNTSCRN";
		break;
	case WXK_INSERT:
		keyname = L"INS";
		break;
	case WXK_HELP:
		keyname = L"HELP";
		break;
	case WXK_NUMPAD0:
		keyname = L"NUM0";
		break;
	case WXK_NUMPAD1:
		keyname = L"NUM1";
		break;
	case WXK_NUMPAD2:
		keyname = L"NUM2";
		break;
	case WXK_NUMPAD3:
		keyname = L"NUM3";
		break;
	case WXK_NUMPAD4:
		keyname = L"NUM4";
		break;
	case WXK_NUMPAD5:
		keyname = L"NUM5";
		break;
	case WXK_NUMPAD6:
		keyname = L"NUM6";
		break;
	case WXK_NUMPAD7:
		keyname = L"NUM7";
		break;
	case WXK_NUMPAD8:
		keyname = L"NUM8";
		break;
	case WXK_NUMPAD9:
		keyname = L"NUM9";
		break;
	case WXK_MULTIPLY:
		keyname = L"MULT";
		break;
	case WXK_ADD:
		keyname = L"ADD";
		break;
	case WXK_SEPARATOR:
		keyname = L"SEP";
		break;
	case WXK_SUBTRACT:
		keyname = L"SUB";
		break;
	case WXK_DECIMAL:
		keyname = L"DEC";
		break;
	case WXK_DIVIDE:
		keyname = L"DIV";
		break;
	case WXK_F1:
		keyname = L"F1";
		break;
	case WXK_F2:
		keyname = L"F2";
		break;
	case WXK_F3:
		keyname = L"F3";
		break;
	case WXK_F4:
		keyname = L"F4";
		break;
	case WXK_F5:
		keyname = L"F5";
		break;
	case WXK_F6:
		keyname = L"F6";
		break;
	case WXK_F7:
		keyname = L"F7";
		break;
	case WXK_F8:
		keyname = L"F8";
		break;
	case WXK_F9:
		keyname = L"F9";
		break;
	case WXK_F10:
		keyname = L"F10";
		break;
	case WXK_F11:
		keyname = L"F11";
		break;
	case WXK_F12:
		keyname = L"F12";
		break;
	case WXK_F13:
		keyname = L"F13";
		break;
	case WXK_F14:
		keyname = L"F14";
		break;
	case WXK_F15:
		keyname = L"F15";
		break;
	case WXK_F16:
		keyname = L"F16";
		break;
	case WXK_F17:
		keyname = L"F17";
		break;
	case WXK_F18:
		keyname = L"F18";
		break;
	case WXK_F19:
		keyname = L"F19";
		break;
	case WXK_F20:
		keyname = L"F20";
		break;
	case WXK_F21:
		keyname = L"F21";
		break;
	case WXK_F22:
		keyname = L"F22";
		break;
	case WXK_F23:
		keyname = L"F23";
		break;
	case WXK_F24:
		keyname = L"F24";
		break;
	case WXK_NUMLOCK:
		keyname = L"NUMLOCK";
		break;
	case WXK_SCROLL:
		keyname = L"SCROLL";
		break;
	case WXK_PAGEUP:
		keyname = L"PAGEUP";
		break;
	case WXK_PAGEDOWN:
		keyname = L"PAGEDOWN";
		break;

	case WXK_NUMPAD_SPACE:
		keyname = L"NUM SPACE";
		break;
	case WXK_NUMPAD_TAB:
		keyname = L"NUM TAB";
		break;
	case WXK_NUMPAD_ENTER:
		keyname = L"NUM ENTER";
		break;
	case WXK_NUMPAD_F1:
		keyname = L"NUM F1";
		break;
	case WXK_NUMPAD_F2:
		keyname = L"NUM F2";
		break;
	case WXK_NUMPAD_F3:
		keyname = L"NUM F3";
		break;
	case WXK_NUMPAD_F4:
		keyname = L"NUM F4";
		break;
	case WXK_NUMPAD_HOME:
		keyname = L"NUM HOME";
		break;
	case WXK_NUMPAD_LEFT:
		keyname = L"NUM LEFT";
		break;
	case WXK_NUMPAD_UP:
		keyname = L"NUM UP";
		break;
	case WXK_NUMPAD_RIGHT:
		keyname = L"NUM RIGHT";
		break;
	case WXK_NUMPAD_DOWN:
		keyname = L"NUM DOWN";
		break;
	case WXK_NUMPAD_PAGEUP:
		keyname = L"NUM PAGEUP";
		break;
	case WXK_NUMPAD_PAGEDOWN:
		keyname = L"NUM PAGEDOWN";
		break;
	case WXK_NUMPAD_END:
		keyname = L"NUM END";
		break;
	case WXK_NUMPAD_BEGIN:
		keyname = L"NUM BEGIN";
		break;
	case WXK_NUMPAD_INSERT:
		keyname = L"NUM INS";
		break;
	case WXK_NUMPAD_DELETE:
		keyname = L"NUM DEL";
		break;
	case WXK_NUMPAD_EQUAL:
		keyname = L"NUM EQUAL";
		break;
	case WXK_NUMPAD_MULTIPLY:
		keyname = L"NUM MULT";
		break;
	case WXK_NUMPAD_ADD:
		keyname = L"NUM ADD";
		break;
	case WXK_NUMPAD_SEPARATOR:
		keyname = L"NUM SEP";
		break;
	case WXK_NUMPAD_SUBTRACT:
		keyname = L"NUM SUB";
		break;
	case WXK_NUMPAD_DECIMAL:
		keyname = L"NUM DEC";
		break;
	case WXK_NUMPAD_DIVIDE:
		keyname = L"NUM DIV";
		break;

	case WXK_WINDOWS_LEFT:
		keyname = L"WIN LEFT";
		break;
	case WXK_WINDOWS_RIGHT:
		keyname = L"WIN RIGHT";
		break;
	case WXK_WINDOWS_MENU:
		keyname = L"WIN MENU";
		break;
//	case WXK_COMMAND:
//		keyname = L"COMMAND";
//		break;

	case WXK_SPECIAL1:
		keyname = L"SPEC1";
		break;
	case WXK_SPECIAL2:
		keyname = L"SPEC2";
		break;
	case WXK_SPECIAL3:
		keyname = L"SPEC3";
		break;
	case WXK_SPECIAL4:
		keyname = L"SPEC4";
		break;
	case WXK_SPECIAL5:
		keyname = L"SPEC5";
		break;
	case WXK_SPECIAL6:
		keyname = L"SPEC6";
		break;
	case WXK_SPECIAL7:
		keyname = L"SPEC7";
		break;
	case WXK_SPECIAL8:
		keyname = L"SPEC8";
		break;
	case WXK_SPECIAL9:
		keyname = L"SPEC9";
		break;
	case WXK_SPECIAL10:
		keyname = L"SPEC10";
		break;
	case WXK_SPECIAL11:
		keyname = L"SPEC11";
		break;
	case WXK_SPECIAL12:
		keyname = L"SPEC12";
		break;
	case WXK_SPECIAL13:
		keyname = L"SPEC13";
		break;
	case WXK_SPECIAL14:
		keyname = L"SPEC14";
		break;
	case WXK_SPECIAL15:
		keyname = L"SPEC15";
		break;
	case WXK_SPECIAL16:
		keyname = L"SPEC16";
		break;
	case WXK_SPECIAL17:
		keyname = L"SPEC17";
		break;
	case WXK_SPECIAL18:
		keyname = L"SPEC18";
		break;
	case WXK_SPECIAL19:
		keyname = L"SPEC19";
		break;
	case WXK_SPECIAL20:
		keyname = L"SPEC20";
		break;
	default:
		keyname = ( boost::wformat( L"%1%" ) % (wchar_t)key ).str();
		break;
	}
	return keyname;
}


void ControlDialog::LabelButton( JOYSTICK_BUTTON button, int key )
{
	guiButtons[ button ]->SetLabel( ( boost::wformat( L"%1% - %2%" ) % GetButtonName( button ) % GetKeyName( key ) ).str().c_str() );
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

