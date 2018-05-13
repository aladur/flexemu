/*
    contpdlg.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#ifndef __contpdlg_h__
#define __contpdlg_h__

#include <wx/dialog.h>

#define IDC_Tracks  101
#define IDC_Sectors 102
#define IDC_Path    103
#define IDC_PathButton  108
#define IDC_FormatCheckBox 109

class wxTextCtrl;
class wxRadioBox;

class ContainerPropertiesDialog : public wxDialog
{
public:
    ContainerPropertiesDialog(wxWindow *parent,
                              const wxPoint &pos, int tracks = 80,
                              int sectors = 40,
                              const wxString &path = wxT(""));
    virtual ~ContainerPropertiesDialog();
    void OnInitDialog(wxInitDialogEvent &event);
    void OnSelectPath(wxCommandEvent &event);
    int GetTracks(void);
    int GetSectors(void);
    inline const wxString &GetPath(void)
    {
        return m_path;
    };
    int GetFormat(void)
    {
        return m_format;
    };

private:
    wxString    m_tracks;
    wxString    m_sectors;
    wxString    m_path;
    int     m_format;

    wxTextCtrl  *c_tracks;
    wxTextCtrl  *c_sectors;
    wxTextCtrl  *c_path;
    wxRadioBox  *c_format;
private:
    DECLARE_EVENT_TABLE()
};

#endif

