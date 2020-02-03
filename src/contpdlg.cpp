/*
    contpdlg.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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


#include "warnoff.h"
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
#include "warnon.h"

#include "misc1.h"

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "contpdlg.h"
#include "mdcrtape.h"
#include "filecnts.h"


#include "warnoff.h"
BEGIN_EVENT_TABLE(ContainerPropertiesDialog, wxDialog)
    EVT_BUTTON(IDC_PathButton,  ContainerPropertiesDialog::OnSelectPath)
    EVT_RADIOBOX(IDC_FormatCheckBox, ContainerPropertiesDialog::OnFormatChanged)
    EVT_COMBOBOX(IDC_DiskFormat, ContainerPropertiesDialog::OnFormatChanged)
    EVT_TEXT(IDC_Tracks, ContainerPropertiesDialog::OnTrkSecChanged)
    EVT_TEXT(IDC_Sectors, ContainerPropertiesDialog::OnTrkSecChanged)
    //  EVT_BUTTON(wxID_OK,  ContainerPropertiesDialog::OnOK)
    //  EVT_BUTTON(wxID_CANCEL, ContainerPropertiesDialog::OnCancel)
END_EVENT_TABLE()
#include "warnon.h"

ContainerPropertiesDialog::ContainerPropertiesDialog(wxWindow *parent,
        const wxPoint &pos /* = wxDefaultPosition */,
        int tracks /* = 80 */,
        int sectors /* = 36 */,
        const wxString &path /* = "" */) :
    wxDialog(parent, 112, _("Create new File Container"), pos),
    m_format(0), c_tracks(nullptr), c_sectors(nullptr),
    c_path(nullptr), c_format(nullptr), c_diskFormat(nullptr)
{
    int diskFormatSelection = 0;
    m_tracks.Printf("%d", tracks);
    m_sectors.Printf("%d", sectors);
    m_path = path;
    int index = 1;
    for (const auto &st : flex_formats)
    {
        if (st.trk == tracks && st.sec == sectors)
        {
            diskFormatSelection = index;
            break;
        }
        ++index;
    }

    wxButton     *pButton;
    wxStaticText *pStatic;

    wxBoxSizer *pMainSizer   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pTrkSecSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pComboBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pWidgetSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pFileSizer   = new wxBoxSizer(wxHORIZONTAL);

    wxString choices[3];
    choices[0] = _("DSK-File");
    choices[1] = _("FLX-File");
    choices[2] = _("MDCR-File");

    c_format = new wxRadioBox(this, IDC_FormatCheckBox, _("Format"),
                              wxDefaultPosition, wxDefaultSize,
                              WXSIZEOF(choices), choices, WXSIZEOF(choices),
                              wxRA_SPECIFY_COLS, wxGenericValidator(&m_format));

    pWidgetSizer->Add(c_format, 0, wxALL, 10);

    pStatic = new wxStaticText(this, -1, _("Disk Format"));
    pComboBoxSizer->Add(pStatic, 0, wxALL, 10);
    c_diskFormat = new wxComboBox(this, IDC_DiskFormat, "",
                                  wxDefaultPosition, wxSize(330, -1), 0,
                                  nullptr, wxCB_READONLY);
    pComboBoxSizer->Add(c_diskFormat, 0, wxALL, 10);

    pWidgetSizer->Add(pComboBoxSizer);

    pStatic = new wxStaticText(this, -1, _("Tracks"));
    pTrkSecSizer->Add(pStatic, 0, wxALL, 10);
    c_tracks = new wxTextCtrl(this, IDC_Tracks, "", wxDefaultPosition,
                              wxDefaultSize, 0,
                              wxTextValidator(wxFILTER_NUMERIC, &m_tracks));
    pTrkSecSizer->Add(c_tracks, 0, wxALL, 10);
    pStatic = new wxStaticText(this, -1, _("Sectors"));
    pTrkSecSizer->Add(pStatic, 0, wxALL, 10);
    c_sectors = new wxTextCtrl(this, IDC_Sectors, "", wxDefaultPosition,
                               wxDefaultSize, 0,
                               wxTextValidator(wxFILTER_NUMERIC, &m_sectors));
    pTrkSecSizer->Add(c_sectors, 0, wxALL, 10);

    pWidgetSizer->Add(pTrkSecSizer);

    c_formatWarning = new wxStaticText(this, -1, _(""));
    c_formatWarning->SetForegroundColour(*wxColour("darkorange"));

    pWidgetSizer->Add(c_formatWarning, 0, wxLEFT, 10);

    pStatic = new wxStaticText(this, -1, _("Filename"));
    pFileSizer->Add(pStatic, 0, wxALL, 10);
    c_path = new wxTextCtrl(this, IDC_Path, "", wxDefaultPosition,
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

    c_diskFormat->Append(_("[Set Tracks and Sectors]"));
    for (const auto *description : flex_format_descriptions)
    {
        c_diskFormat->Append(description);
    }
    c_diskFormat->SetSelection(diskFormatSelection);
    c_tracks->Enable(diskFormatSelection == 0);
    c_sectors->Enable(diskFormatSelection == 0);
}

ContainerPropertiesDialog::~ContainerPropertiesDialog()
{
}

void ContainerPropertiesDialog::OnSelectPath(wxCommandEvent &WXUNUSED(event))
{
    wxString containerPath;
#ifdef WIN32
    char                    wd[PATH_MAX];

    getcwd(wd, PATH_MAX);
#endif
    containerPath = wxFileSelector(
                        _("Input a FLEX file container"),
                        "",
                        "",
                        _("*.DSK"),
                        _("*.*"),
                        wxFD_SAVE,
                        this);
#ifdef WIN32
    chdir(wd);
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

void ContainerPropertiesDialog::OnFormatChanged(wxCommandEvent &WXUNUSED(event))
{
    auto selection = c_diskFormat->GetSelection();
    bool isMdcrFormat = (c_format->GetSelection() == 2);
    bool isFreeDiskFormat = (selection == 0);

    if (selection > 0)
    {
        auto trk_sec = flex_formats[selection - 1];
        m_tracks.Printf("%d", static_cast<int>(trk_sec.trk));
        m_sectors.Printf("%d", static_cast<int>(trk_sec.sec));
        c_tracks->SetValue(m_tracks);
        c_sectors->SetValue(m_sectors);
    }

    c_diskFormat->Enable(!isMdcrFormat);
    c_tracks->Enable(!isMdcrFormat && isFreeDiskFormat);
    c_sectors->Enable(!isMdcrFormat && isFreeDiskFormat);
}

int ContainerPropertiesDialog::GetTracks() const
{
    long t;
    if (c_tracks->GetValue().ToLong(&t))
    {
        return (int)t;
    }
    else
    {
        return 0;
    }
}

int ContainerPropertiesDialog::GetSectors() const
{
    long s;
    if (c_sectors->GetValue().ToLong(&s))
    {
        return (int)s;
    }
    else
    {
        return 0;
    }
}

const wxString ContainerPropertiesDialog::GetPath() const
{
    return c_path->GetValue();
}

void ContainerPropertiesDialog::OnTrkSecChanged(wxCommandEvent &WXUNUSED(event))
{
    auto selection = c_diskFormat->GetSelection();
    bool isFreeDiskFormat = (selection == 0);
    bool isWarning = false;

    if (isFreeDiskFormat)
    {
        auto tracks = GetTracks();
        auto sectors = GetSectors();

        isWarning = true;
        for (const auto &st : flex_formats)
        {
            if (st.trk == tracks && st.sec == sectors)
            {
                isWarning = false;
                break;
            }
        }
    }

    c_formatWarning->SetLabelText(isWarning ?
                                  _("Uncommon FLEX disk format. "
                                    "This may cause compatibility "
                                    "issues!") : "");
}

bool ContainerPropertiesDialog::IsTracksValid(int tracks)
{
        return tracks >= 2 && tracks <= 255;
}

bool ContainerPropertiesDialog::IsSectorsValid(int sectors)
{
        return sectors >= 6 && sectors <= 255;
}

bool ContainerPropertiesDialog::Validate()
{
    if (!IsTracksValid(GetTracks()))
    {
        wxMessageBox(_("Invalid track number. Valid range: [2..255]."),
                     _("FLEXplorer Error"),
                     wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return false;
    }

    if (!IsSectorsValid(GetSectors()))
    {
        wxMessageBox(_("Invalid sector number. Valid range: [6..255]."),
                     _("FLEXplorer Error"),
                     wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return false;
    }

    if (GetPath().IsEmpty())
    {
        wxMessageBox(_("No path specified") , _("FLEXplorer Error"),
                wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return false;
    }

    return wxWindow::Validate();
}

