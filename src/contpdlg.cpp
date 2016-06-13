/*
    contpdlg.cpp


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


#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/radiobox.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include <misc1.h>

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "contpdlg.h"


BEGIN_EVENT_TABLE(ContainerPropertiesDialog, wxDialog)
    EVT_BUTTON(IDC_PathButton,  ContainerPropertiesDialog::OnSelectPath)
    //  EVT_BUTTON(wxID_OK,  ContainerPropertiesDialog::OnOK)
    //  EVT_BUTTON(wxID_CANCEL, ContainerPropertiesDialog::OnCancel)
END_EVENT_TABLE()

ContainerPropertiesDialog::ContainerPropertiesDialog(wxWindow *parent,
        const wxPoint &pos /* = wxDefaultPosition */,
        int tracks /* = 80 */,
        int sectors /* = 40 */,
        const wxString &path /* = wxT("") */) :
    wxDialog(parent, 112, _("Create new File Container"), pos),
    m_format(0), c_tracks(NULL), c_sectors(NULL),
    c_path(NULL), c_format(NULL)
{
    m_tracks.Printf(wxT("%d"), tracks);
    m_sectors.Printf(wxT("%d"), sectors);
    m_path = path;

    wxButton     *pButton;
    wxStaticText *pStatic;

    wxBoxSizer *pMainSizer   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pTrkSecSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pWidgetSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pFileSizer   = new wxBoxSizer(wxHORIZONTAL);

    wxString choices[2];
    choices[0] = _("DSK-File");
    choices[1] = _("FLX-File");

    c_format = new wxRadioBox(this, IDC_FormatCheckBox, _("Format"),
                              wxDefaultPosition, wxDefaultSize, 2, choices, 2,
                              wxRA_SPECIFY_COLS, wxGenericValidator(&m_format));

    pWidgetSizer->Add(c_format, 0, wxALL, 10);

    pStatic = new wxStaticText(this, -1, _("Tracks"));
    pTrkSecSizer->Add(pStatic, 0, wxALL, 10);
    c_tracks = new wxTextCtrl(this, IDC_Tracks, wxT(""), wxDefaultPosition,
                              wxDefaultSize, 0,
                              wxTextValidator(wxFILTER_NUMERIC, &m_tracks));
    pTrkSecSizer->Add(c_tracks, 0, wxALL, 10);
    pStatic = new wxStaticText(this, -1, _("Sectors"));
    pTrkSecSizer->Add(pStatic, 0, wxALL, 10);
    c_sectors = new wxTextCtrl(this, IDC_Sectors, wxT(""), wxDefaultPosition,
                               wxDefaultSize, 0,
                               wxTextValidator(wxFILTER_NUMERIC, &m_sectors));
    pTrkSecSizer->Add(c_sectors, 0, wxALL, 10);

    pWidgetSizer->Add(pTrkSecSizer);

    pStatic = new wxStaticText(this, -1, _("Filename"));
    pFileSizer->Add(pStatic, 0, wxALL, 10);
    c_path = new wxTextCtrl(this, IDC_Path, wxT(""), wxDefaultPosition,
                            wxSize(200, -1), 0,
                            wxTextValidator(wxFILTER_NONE, &m_path));
    pFileSizer->Add(c_path, 0, wxALL, 10);
    pButton = new wxButton(this, IDC_PathButton, _("..."),
                           wxDefaultPosition, wxSize(40, -1));
    pFileSizer->Add(pButton, 0, wxALL, 10);

    pWidgetSizer->Add(pFileSizer);

    pMainSizer->Add(pWidgetSizer);

    pButton = new wxButton(this, wxID_OK, _("&Ok"));
    pButtonSizer->Add(pButton, 0, wxALL, 5);
    pButton = new wxButton(this, wxID_CANCEL, _("&Cancel"));
    pButtonSizer->Add(pButton, 0, wxALL, 5);

    pMainSizer->Add(pButtonSizer);

    SetSizer(pMainSizer);
    pMainSizer->SetSizeHints(this);
    c_path->SetFocus();
}

ContainerPropertiesDialog::~ContainerPropertiesDialog()
{
}

void ContainerPropertiesDialog::OnSelectPath(wxCommandEvent &WXUNUSED(event))
{
    wxString containerPath;
#ifdef WIN32
    char                    wd[PATH_MAX];

    getcwd((char *)wd, PATH_MAX);
#endif
    containerPath = wxFileSelector(
                        _("Input a FLEX file container"),
                        wxT(""),
                        wxT(""),
                        _("*.DSK"),
                        _("*.*"),
                        wxFD_SAVE,
                        this);
#ifdef WIN32
    chdir((char *)wd);
#endif

    if (!containerPath.IsEmpty())
    {
        m_path = containerPath;

        if (c_path)
        {
            c_path->SetValue(m_path);
        }
    }
}

int ContainerPropertiesDialog::GetTracks(void)
{
    long t;

    if (m_tracks.ToLong(&t))
    {
        return (int)t;
    }
    else
    {
        return 0;
    }
}

int ContainerPropertiesDialog::GetSectors(void)
{
    long s;

    if (m_tracks.ToLong(&s))
    {
        return (int)s;
    }
    else
    {
        return 0;
    }
}

