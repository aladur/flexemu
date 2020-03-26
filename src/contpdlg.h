/*
    contpdlg.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2020  W. Schwotzer

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

#ifndef CONTPDLG_INCLUDED
#define CONTPDLG_INCLUDED

#include <wx/dialog.h>

#define IDC_Tracks  101
#define IDC_Sectors 102
#define IDC_Path    103
#define IDC_PathButton  108
#define IDC_FormatCheckBox 109
#define IDC_DiskFormat 110

class wxTextCtrl;
class wxRadioBox;
class wxComboBox;
class wxStaticText;

class ContainerPropertiesDialog : public wxDialog
{
public:
    ContainerPropertiesDialog(wxWindow *parent,
                              const wxPoint &pos, int tracks = 80,
                              int sectors = 36,
                              const wxString &path = "");
    virtual ~ContainerPropertiesDialog();
    void OnInitDialog(wxInitDialogEvent &event);
    void OnSelectPath(wxCommandEvent &event);
    void OnFormatChanged(wxCommandEvent &event);
    void OnTrkSecChanged(wxCommandEvent &event);
    int GetTracks() const;
    int GetSectors() const;
    const wxString GetPath() const;
    int GetFormat()
    {
        return m_format;
    };

private:
    static bool IsTracksValid(int tracks);
    static bool IsSectorsValid(int sectors);
    bool Validate();

    wxString    m_tracks;
    wxString    m_sectors;
    wxString    m_path;
    int     m_format;

    wxTextCtrl  *c_tracks;
    wxTextCtrl  *c_sectors;
    wxTextCtrl  *c_path;
    wxRadioBox  *c_format;
    wxComboBox  *c_diskFormat;
    wxStaticText  *c_formatWarning;

private:
    DECLARE_EVENT_TABLE()
};

#endif

