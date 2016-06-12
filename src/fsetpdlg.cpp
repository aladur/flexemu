/*
    fsetpdlg.cpp


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


// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif
#include <wx/combobox.h>
#include <wx/radiobox.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include <misc1.h>

#ifdef _MSC_VER
#include <direct.h>
#endif

#include "fsetpdlg.h"
#include "fsetup.h"

const char *color_table[15] = {
	"default", "white", "red", "green", "blue", "yellow", "magenta",
	"cyan", "orange", "pink", "purple", "violet", "orange", "brown",
	NULL };

int possible_nColors[4] = { 2, 8, 64, 0 };

BEGIN_EVENT_TABLE(FlexemuOptionsDialog, wxDialog)
	EVT_BUTTON(IDC_Drive0Button,  FlexemuOptionsDialog::OnSelectDrive0)
	EVT_BUTTON(IDC_Drive1Button,  FlexemuOptionsDialog::OnSelectDrive1)
	EVT_BUTTON(IDC_Drive2Button,  FlexemuOptionsDialog::OnSelectDrive2)
	EVT_BUTTON(IDC_Drive3Button,  FlexemuOptionsDialog::OnSelectDrive3)
	EVT_BUTTON(IDC_DiskDirButton, FlexemuOptionsDialog::OnSelectDiskDir)
	EVT_BUTTON(IDC_MonitorButton, FlexemuOptionsDialog::OnSelectMonitor)
	EVT_BUTTON(wxID_OK,           FlexemuOptionsDialog::OnOkCancel)
	EVT_BUTTON(wxID_CANCEL,       FlexemuOptionsDialog::OnOkCancel)
	EVT_INIT_DIALOG(              FlexemuOptionsDialog::OnInitDialog)
	EVT_CLOSE(FlexemuOptionsDialog::OnCloseWindow)
END_EVENT_TABLE()

FlexemuOptionsDialog::FlexemuOptionsDialog(
                 struct sGuiOptions *pGuiOptions,
                 struct sOptions *pOptions,
		wxWindow* parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& name) :
	wxDialog(parent, id, title, pos, size, style, name),
	m_guiOptions(pGuiOptions), m_options(pOptions),
	c_color(NULL), c_finverse(NULL), c_undocumented(NULL),
	c_geometry(NULL), c_nColors(NULL), c_monitor(NULL),
	c_wwwBrowser(NULL), c_diskDir(NULL), 
	c_ramExtension(NULL)
{
	int i;

	for (i = 0; i < 4; ++i)
		c_drive[i] = NULL;
}

FlexemuOptionsDialog::~FlexemuOptionsDialog()
{
}

bool FlexemuOptionsDialog::TransferDataToWindow(void)
{
	BString str;
	int x, y;
	int n = 0;

	if (c_geometry) {
	   for (x = 1; x <= MAX_GUIXSIZE; x++)
		for (y = 1; y <= MAX_GUIYSIZE; y++) {
			str.printf("%ix%i", WINDOWWIDTH * x, WINDOWHEIGHT * y);
			if (c_geometry) c_geometry->Append((const char *)str);
			if (m_guiOptions->guiXSize == x &&
			   m_guiOptions->guiYSize == y)
					c_geometry->SetSelection(n);
			n++;
		}
	}
	n = 0;
	if (c_nColors) {
		int *pInt = possible_nColors;
		while (*pInt) {
			str.printf("%d", *pInt);
			c_nColors->Append((const char *)str);
			if (m_guiOptions->nColors == *(pInt++))
				c_nColors->SetSelection(n);
			n++;
		}
	}
	n = 0;
	if (c_color) {
		const char **p = color_table;
		while (*p) {
			c_color->Append(*p);
			if (!m_guiOptions->color.comparenocase(*(p++)))
				c_color->SetSelection(n);
			n++;
		}
	}
	if (c_finverse)  c_finverse->SetValue(m_guiOptions->inverse != 0);
	if (c_undocumented)  c_undocumented->SetValue(m_options->use_undocumented);
	if (c_monitor)  c_monitor->SetValue((const char *)m_options->hex_file);
	if (c_wwwBrowser)  c_wwwBrowser->SetValue(
		(const char *)m_guiOptions->www_browser);
	if (c_diskDir)  c_diskDir->SetValue((const char *)m_options->disk_dir);
	for (x=0; x<=3; x++)
		if (c_drive[x]) c_drive[x]->SetValue(
			(const char *)m_options->drive[x]);
	if (c_ramExtension)
		c_ramExtension->SetSelection(m_options->isHiMem ? 1 : 0);

	return true;
}

void FlexemuOptionsDialog::OnInitDialog(wxInitDialogEvent &event)
{
	wxButton	*pButton;
	wxStaticText	*pStatic;
	int		i;
	const int	gap = 5;
	const int	textWidth  = 250;
	const int	stextWidth = 140;
	wxWindow	*parent = this;
	wxString	text;
	wxString	choices[2];
	wxBoxSizer	*pBoxSizer;

	wxBoxSizer  *pMainSizer   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer  *pButtonSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer  *pWidgetSizer = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *pGrid1Sizer  = new wxFlexGridSizer(2, 3);

	pStatic = new wxStaticText(parent, -1, "Screen geometry",
		wxDefaultPosition, wxSize(stextWidth, -1));
	pGrid1Sizer->Add(pStatic, 0, wxRIGHT, gap);
	c_geometry = new wxComboBox(parent, IDC_Geometry, "", wxDefaultPosition,
		wxDefaultSize, 0, choices, wxCB_READONLY);
	pGrid1Sizer->Add(c_geometry, 0, wxEXPAND);

	pStatic = new wxStaticText(parent, -1, _T("Number of Colors"),
		wxDefaultPosition, wxSize(stextWidth, -1));
	pGrid1Sizer->Add(pStatic, 0, wxRIGHT, gap);
	c_nColors = new wxComboBox(parent, IDC_nColors, "", wxDefaultPosition,
		wxDefaultSize, 0, choices, wxCB_READONLY);
	pGrid1Sizer->Add(c_nColors, 0, wxEXPAND);

	pStatic = new wxStaticText(parent, -1, _T("Color"),
		wxDefaultPosition, wxSize(stextWidth, -1));
	pGrid1Sizer->Add(pStatic, 0, wxRIGHT, gap);
	c_color = new wxComboBox(parent, IDC_Color, "", wxDefaultPosition,
		wxDefaultSize, 0, choices, wxCB_READONLY);
	pGrid1Sizer->Add(c_color);

	pWidgetSizer->Add(pGrid1Sizer, 0, wxEXPAND);

	c_finverse = new wxCheckBox(parent, IDC_Inverse,
                        _T("Display inverse"), wxDefaultPosition,
                        wxDefaultSize, 0);
	pWidgetSizer->Add(c_finverse, 0, wxEXPAND | wxBOTTOM, gap);

	c_undocumented = new wxCheckBox(parent, IDC_Inverse,
                        _T("Undocumented MC6809 instructions"),
			wxDefaultPosition, wxDefaultSize, 0);
	pWidgetSizer->Add(c_undocumented, 0, wxEXPAND | wxBOTTOM, gap);

	choices[0] = _T("2 x 96 KByte");
	choices[1] = _T("2 x 288 KByte");

	c_ramExtension = new wxRadioBox(parent, IDC_RamExtension,
	_T("RAM extension"),
		wxDefaultPosition, wxDefaultSize, 2, choices, 2,
		wxRA_SPECIFY_COLS);

	pWidgetSizer->Add(c_ramExtension, 0, wxEXPAND | wxBOTTOM, gap);

#ifndef WIN32
	pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pStatic = new wxStaticText(parent, -1, _T("HTML Browser"),
		wxDefaultPosition, wxSize(stextWidth, -1));
	pBoxSizer->Add(pStatic, 0, wxCENTER);
	c_wwwBrowser = new wxTextCtrl(parent, IDC_WwwBrowser, "", wxDefaultPosition,
			wxSize(textWidth, -1), 0);
	pBoxSizer->Add(c_wwwBrowser, 1, wxCENTER | wxRIGHT, gap);
	pStatic = new wxStaticText(parent, -1, "", wxDefaultPosition,
		wxSize(40, -1));
	pBoxSizer->Add(pStatic, 0, wxCENTER); // dummy widget instead of a Button
	pWidgetSizer->Add(pBoxSizer, 0, wxEXPAND);
#endif
	pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pStatic = new wxStaticText(parent, -1, _T("Disk/Monitor directory"),
		wxDefaultPosition, wxSize(stextWidth, -1));
	pBoxSizer->Add(pStatic, 0, wxCENTER);
	c_diskDir = new wxTextCtrl(parent, IDC_DiskDir, "", wxDefaultPosition,
			wxSize(textWidth, -1));
	pBoxSizer->Add(c_diskDir, 1, wxCENTER | wxRIGHT, gap);
	pButton = new wxButton(parent, IDC_DiskDirButton, _T("..."),
		wxDefaultPosition, wxSize(40, -1));
	pBoxSizer->Add(pButton, 0, wxCENTER);
	pWidgetSizer->Add(pBoxSizer, 0, wxEXPAND);

	pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	pStatic = new wxStaticText(parent, -1, _T("Monitor program"),
		wxDefaultPosition, wxSize(stextWidth, -1));
	pBoxSizer->Add(pStatic, 0, wxCENTER);
	c_monitor = new wxTextCtrl(parent, IDC_Monitor, "", wxDefaultPosition,
			wxSize(textWidth, -1));
	pBoxSizer->Add(c_monitor, 1, wxCENTER | wxRIGHT, gap);
	pButton = new wxButton(parent, IDC_MonitorButton, _T("..."),
		wxDefaultPosition, wxSize(40, -1));
	pBoxSizer->Add(pButton, 0, wxCENTER);
	pWidgetSizer->Add(pBoxSizer, 0, wxEXPAND);

	for (i = 0; i < 4; ++i)
	{
		pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		text.Printf("Disk for Drive %d", i);
		pStatic = new wxStaticText(parent, -1, text,
			wxDefaultPosition, wxSize(stextWidth, -1));
		pBoxSizer->Add(pStatic, 0, wxCENTER);
		c_drive[i] = new wxTextCtrl(parent, IDC_Drive0+i, "", wxDefaultPosition,
				wxSize(textWidth, -1));
		pBoxSizer->Add(c_drive[i], 1, wxCENTER | wxRIGHT, gap);
		pButton = new wxButton(parent, IDC_Drive0Button+i, _T("..."),
			wxDefaultPosition, wxSize(40, -1));
		pBoxSizer->Add(pButton, 0, wxCENTER);
		pWidgetSizer->Add(pBoxSizer, 0, wxEXPAND);
	}

	pMainSizer->Add(pWidgetSizer, 1, wxALL, gap);

	pButton = new wxButton(parent, wxID_OK, _T("&Ok"));
	pButtonSizer->Add(pButton, 0, wxCENTER | wxALL, gap);
	pButton = new wxButton(parent, wxID_CANCEL, _T("&Cancel"));
	pButtonSizer->Add(pButton, 0, wxCENTER | wxALL, gap);

	pMainSizer->Add(pButtonSizer);

	SetAutoLayout(true);
	SetSizer( pMainSizer );
	pMainSizer->Fit(this);
	pMainSizer->SetSizeHints( this );
	Centre(wxBOTH);
	//c_geometry->SetFocus();

	wxDialog::OnInitDialog(event); // must be last command
}

void FlexemuOptionsDialog::OnCloseWindow(wxCloseEvent &event)
{
	if (!event.CanVeto())
		Destroy();
	else
		wxDialog::OnCloseWindow(event);
}

void FlexemuOptionsDialog::OnOkCancel(wxCommandEvent& event )
{
	switch (event.GetId())
	{
	case wxID_OK:
		RestoreValues();
		// doing some verification of the values
		if (m_options->hex_file.empty())
		{
			wxMessageBox("Monitor program must not be empty",
			   "FSetup Error", wxOK | wxCENTRE | wxICON_EXCLAMATION);
		} else
			wxDialog::OnOK(event);
		break;
	case wxID_CANCEL:
		wxDialog::OnCancel(event);
		break;
	default: return;
	}

	wxGetApp().SetReturnCode(GetReturnCode());
	if (!IsModal())
		Close(true);
}

void FlexemuOptionsDialog::RestoreValues(void)
{

	if (c_geometry) {
		wxString geometry;
		int x, y;

		geometry = c_geometry->GetValue();
		if (sscanf(geometry, "%dx%d", &x, &y) == 2) {
		   m_guiOptions->guiXSize = x / WINDOWWIDTH;
		   m_guiOptions->guiYSize = y / WINDOWHEIGHT;
		}
	}
	if (c_nColors) {
		wxString nrOfColors;
		int n;

		nrOfColors = c_nColors->GetValue();
		if (sscanf(nrOfColors, "%d", &n) == 1) {
		   m_guiOptions->nColors = n;
		}
	}
	if (c_color) { m_guiOptions->color = c_color->GetValue(); };
	if (c_finverse) { m_guiOptions->inverse = c_finverse->GetValue(); };
	if (c_undocumented) { m_options->use_undocumented = (c_undocumented->GetValue() != 0); };
	if (c_monitor) { m_options->hex_file = c_monitor->GetValue(); };
#ifndef WIN32
	if (c_wwwBrowser) { m_guiOptions->www_browser =
		c_wwwBrowser->GetValue(); };
#endif
	if (c_diskDir) { m_options->disk_dir = c_diskDir->GetValue(); };
	for (int i=0; i<=3; i++)
		if (c_drive[i])
			m_options->drive[i] = c_drive[i]->GetValue();
	if (c_ramExtension)
		{ m_options->isHiMem = c_ramExtension->GetSelection() > 0; };
}

BString FlexemuOptionsDialog::OpenFilePrompter(
	const char *defautPath,
	const char *caption,
	const char *filter)
{
	wxString drive;
	BString bdrive;

#ifdef WIN32
	char                    wd[PATH_MAX];

	getcwd((char *)wd, PATH_MAX);
#endif
	drive = wxFileSelector(
		"Select a disk",
		defautPath,
		NULL,
		"*.DSK",
		"*.*",
		wxSAVE,
		this);
#ifdef WIN32
	chdir((char *)wd);
#endif
	if (!drive.IsEmpty())
		bdrive = (const char *)drive;
	return bdrive;
}

void FlexemuOptionsDialog::OnSelectDriveN(int n)
{
	BString path;
	BString diskDir;

	if (n > 3)
		return;
	if (c_diskDir) { diskDir = c_diskDir->GetValue(); };
	path = OpenFilePrompter(diskDir, "Select a Disk file", "*.DSK");
	if (diskDir.length() > 0 &&
			(path.index(diskDir) == 0) &&
			(path.index(PATHSEPARATOR) >= 0)) {
		path.at(diskDir.length(), path.length()-diskDir.length(), path);
		if (path.index(PATHSEPARATOR) == 0)
			path.at(1, path.length()-1, path);
	}
	if (!path.empty())
		if (c_drive[n]) c_drive[n]->SetValue((const char *)path);
}

void FlexemuOptionsDialog::OnSelectDrive0(wxCommandEvent& WXUNUSED(event) )
{
	OnSelectDriveN(0);
}

void FlexemuOptionsDialog::OnSelectDrive1(wxCommandEvent& WXUNUSED(event) )
{
	OnSelectDriveN(1);
}

void FlexemuOptionsDialog::OnSelectDrive2(wxCommandEvent& WXUNUSED(event) )
{
	OnSelectDriveN(2);
}

void FlexemuOptionsDialog::OnSelectDrive3(wxCommandEvent& WXUNUSED(event) )
{
	OnSelectDriveN(3);
}

void FlexemuOptionsDialog::OnSelectDiskDir(wxCommandEvent& WXUNUSED(event) )
{
    wxDirDialog *dialog;

	if (c_diskDir) { m_options->disk_dir = c_diskDir->GetValue(); };
    dialog = new wxDirDialog(this, "Select folder with DSK files",
		(const char *)m_options->disk_dir);
	if (dialog->ShowModal() == wxID_OK) {
		m_options->disk_dir = dialog->GetPath();
		if (c_diskDir)
			c_diskDir->SetValue((const char *)m_options->disk_dir);
	}
	delete dialog;
}

void FlexemuOptionsDialog::OnSelectMonitor(wxCommandEvent& WXUNUSED(event))
{
	BString path;
	BString diskDir;

	if (c_diskDir) { diskDir = c_diskDir->GetValue(); };
	path = OpenFilePrompter(diskDir, "Select a monitor program", "*.HEX");
	if (diskDir.length() > 0 &&
			(path.index(diskDir) == 0) &&
			(path.index(PATHSEPARATOR) >= 0)) {
		path.at(diskDir.length(), path.length()-diskDir.length(), path);
		if (path.index(PATHSEPARATOR) == 0)
			path.at(1, path.length()-1, path);
	}
	if (!path.empty())
		if (c_monitor) c_monitor->SetValue((const char *)path);
}
