/*
    fsetpdlg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __fsetpdlg_h__
#define __fsetpdlg_h__

#include <wx/dialog.h>

#include "misc1.h"
#include "bstring.h"
#include "absgui.h"
#include "brcfile.h"
#include "bregistr.h"

class wxCheckBox;
class wxTextCtrl;
class wxComboBox;
class wxRadioBox;


enum {
	IDC_Geometry       = 200,
	IDC_Color          = 201,
	IDC_Inverse        = 202,
	IDC_WwwBrowser     = 203,
	IDC_DiskDir        = 204,
	IDC_Monitor        = 205,
	IDC_Drive0         = 206,
	IDC_Drive1         = 207,
	IDC_Drive2         = 208,
	IDC_Drive3         = 209,
	IDC_nColors        = 210,
	IDC_RamExtension   = 211,
	IDC_Undocumented   = 212,

	IDC_DiskDirButton  = 304,
	IDC_MonitorButton  = 305,
	IDC_Drive0Button   = 306,
	IDC_Drive1Button   = 307,
	IDC_Drive2Button   = 308,
	IDC_Drive3Button   = 309
};

class FlexemuOptionsDialog : public wxDialog {
public:

	FlexemuOptionsDialog(
                 struct sGuiOptions *pGuiOptions,
                 struct sOptions *pOptions,
		wxWindow* parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE,
		const wxString& name = "FlexemuOptionsDialog");
	virtual ~FlexemuOptionsDialog();

	void    OnInitDialog(wxInitDialogEvent &event);
	void    OnCloseWindow(wxCloseEvent &event);
	void    OnOkCancel(wxCommandEvent& event);
	bool	TransferDataToWindow(void);
	void    RestoreValues(void);
	BString OpenFilePrompter(
			const char *defautPath = "",
			const char *caption = "",
			const char *filter = "*.*");
	void    OnSelectDriveN(int n);
	void    OnSelectDrive0(wxCommandEvent& WXUNUSED(event));
	void    OnSelectDrive1(wxCommandEvent& WXUNUSED(event));
	void    OnSelectDrive2(wxCommandEvent& WXUNUSED(event));
	void    OnSelectDrive3(wxCommandEvent& WXUNUSED(event));
	void    OnSelectDiskDir(wxCommandEvent& WXUNUSED(event));
	void    OnSelectMonitor(wxCommandEvent& WXUNUSED(event));

private:
	FlexemuOptionsDialog() { }; // should not be used

	struct sGuiOptions *m_guiOptions;
	struct sOptions *m_options;

	wxComboBox *c_color;
	wxCheckBox *c_finverse;
	wxCheckBox *c_undocumented;
	wxComboBox *c_geometry;
	wxComboBox *c_nColors;
	wxTextCtrl *c_monitor;
	wxTextCtrl *c_wwwBrowser;
	wxTextCtrl *c_diskDir;
	wxTextCtrl *c_drive[4];
	wxRadioBox *c_ramExtension;

	DECLARE_EVENT_TABLE()
};

#endif

