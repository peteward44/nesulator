
#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H


#include "gui_shared.h"
//#include "../mainboard.h"
#include "wx/radiobox.h"


class ConfigControlsDialog : public wxDialog
{
private:
	DECLARE_EVENT_TABLE();

	wxButton* configButtons[2];
	wxRadioBox* radioBoxes[2];

	void EnableConfigButton( int controller );

public:
	ConfigControlsDialog( wxWindow* parent );

	void OnCloseButton( wxCommandEvent& ev );
	void OnRadioBoxChange( wxCommandEvent& ev );
	void OnConfigKeysButton( wxCommandEvent& ev );
};


#endif

