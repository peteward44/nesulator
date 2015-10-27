
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H


#include "gui_shared.h"
#include "wx/radiobox.h"


class OptionsDialog : public wxDialog
{
private:
	wxButton* configButtons[2];
	wxRadioBox* radioBoxes[2];

	wxRadioBox* mFilteringRadioBox;

	void EnableConfigButton( int controller );

	DECLARE_EVENT_TABLE();

	wxCheckBox* cbDisableFrameLimiting, *cbAllowSprites, *cbEnableSound;
	wxSlider* slVolume;
	wxTextCtrl *tcSaveState, *tcScreenshot;

public:
	OptionsDialog( wxWindow* parent );

	void OnOkButton( wxCommandEvent& ev );
	void OnCancelButton( wxCommandEvent& ev );
	void OnRadioBoxChange( wxCommandEvent& ev );
	void OnConfigKeysButton( wxCommandEvent& ev );
	void OnDirectoryBrowseButton( wxCommandEvent& ev );
};


#endif

