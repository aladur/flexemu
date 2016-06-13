/*
    optdlg.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

#ifndef __optdlg_h__
#define __optdlg_h__

#include <wx/dialog.h>

enum
{
    IDC_AutoTextFlag         = 202,
    IDC_Viewer               = 203,
    IDC_ViewerButton         = 204,
    IDC_BootSectorFile       = 205,
    IDC_BootSectorFileButton = 206
};


class wxTextCtrl;
class wxCheckBox;

class GlobalOptionsDialog : public wxDialog
{
public:
    GlobalOptionsDialog(wxWindow *pParent,
                        const wxPoint &pos = wxDefaultPosition,
                        const bool autoTextFlag = false,
                        wxString bootFile = wxT("boot"),
                        wxString editor = wxT(""));
    virtual ~GlobalOptionsDialog();
    void OnSelectViewer(wxCommandEvent &event);
    void OnSelectBootSectorFile(wxCommandEvent &event);
    inline bool GetAutoTextFlag(void)
    {
        return m_autoTextFlag;
    };
    inline wxString GetViewer(void)
    {
        return m_viewer;
    };
    inline wxString GetBootSectorFile(void)
    {
        return m_bootSectorFile;
    };

private:
    wxString    m_viewer;
    wxString    m_bootSectorFile;
    bool        m_autoTextFlag;

    wxTextCtrl      *c_viewer;
    wxTextCtrl      *c_bootSectorFile;
    wxCheckBox      *c_autoTextFlag;
private:
    DECLARE_EVENT_TABLE()
};

#endif

