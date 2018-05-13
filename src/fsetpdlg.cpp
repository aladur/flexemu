/*
    fsetpdlg.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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
#include <wx/treebook.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "misc1.h"
#include <string>

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "fsetpdlg.h"
#include "fsetup.h"


static const wxChar *color_table[] =
{
    _("default"),
    _("white"),
    _("red"),
    _("green"),
    _("blue"),
    _("yellow"),
    _("magenta"),
    _("cyan"),
    _("orange"),
    _("pink"),
    _("purple"),
    _("violet"),
    _("brown"),
};

int possible_nColors[4] = { 2, 8, 64, 0 };

BEGIN_EVENT_TABLE(FlexemuOptionsDialog, wxDialog)
    EVT_BUTTON(IDC_Drive0Button, FlexemuOptionsDialog::OnSelectDrive0)
    EVT_BUTTON(IDC_Drive1Button, FlexemuOptionsDialog::OnSelectDrive1)
    EVT_BUTTON(IDC_Drive2Button, FlexemuOptionsDialog::OnSelectDrive2)
    EVT_BUTTON(IDC_Drive3Button, FlexemuOptionsDialog::OnSelectDrive3)
    EVT_BUTTON(IDC_DiskDirButton, FlexemuOptionsDialog::OnSelectDiskDir)
    EVT_BUTTON(IDC_MonitorButton, FlexemuOptionsDialog::OnSelectMonitor)
    EVT_INIT_DIALOG(FlexemuOptionsDialog::OnInitDialog)
    //  EVT_CLOSE(FlexemuOptionsDialog::OnCloseWindow)
END_EVENT_TABLE()

const int FlexemuOptionsDialog::textWidth = 250;
const int FlexemuOptionsDialog::stextWidth = 150;
const int FlexemuOptionsDialog::gap = 5;

FlexemuOptionsDialog::FlexemuOptionsDialog(
    struct sGuiOptions *pGuiOptions,
    struct sOptions *pOptions,
    wxWindow *parent,
    wxWindowID id,
    const wxString &title,
    const wxPoint &pos,
    const wxSize &size,
    long style,
    const wxString &name) :
    wxDialog(parent, id, title, pos, size, style, name),
    m_guiOptions(pGuiOptions), m_options(pOptions),
    c_color(NULL), c_isInverse(NULL), c_undocumented(NULL),
    c_geometry(NULL), c_nColors(NULL), c_monitor(NULL),
    c_htmlViewer(NULL), c_diskDir(NULL),
    c_ramExtension(NULL)
{
    int i;

    for (i = 0; i < 4; ++i)
    {
        c_drive[i] = NULL;
    }
}

FlexemuOptionsDialog::~FlexemuOptionsDialog()
{
}

bool FlexemuOptionsDialog::TransferDataToWindow(void)
{
    wxString str;
    int x;
    int n = 0;

    if (c_geometry)
    {
        for (x = 1; x <= MAX_PIXELSIZEX; x++)
        {
            int y;

            for (y = 1; y <= MAX_PIXELSIZEY; y++)
            {
                str.Printf(wxT("%ix%i"), WINDOWWIDTH * x, WINDOWHEIGHT * y);

                if (c_geometry)
                {
                    c_geometry->Append(str);
                }

                if (m_guiOptions->pixelSizeX == x &&
                    m_guiOptions->pixelSizeY == y)
                {
                    c_geometry->SetSelection(n);
                }

                n++;
            }
        }
    }

    n = 0;

    if (c_nColors)
    {
        int *pInt = possible_nColors;

        while (*pInt)
        {
            str.Printf(wxT("%d"), *pInt);
            c_nColors->Append(str);

            if (m_guiOptions->nColors == *(pInt++))
            {
                c_nColors->SetSelection(n);
            }

            n++;
        }
    }

    if (c_color)
    {
        unsigned int i;
        wxString colorName;
        std::string bColorName;

        for (i = 0; i < WXSIZEOF(color_table); i++)
        {
            colorName = wxGetTranslation(color_table[i]);
            c_color->Append(colorName);
            wxString color(color_table[i], *wxConvCurrent);
            std::string sColorName(color.mb_str(*wxConvCurrent));

            if (!stricmp(m_guiOptions->color.c_str(), sColorName.c_str()))
            {
                c_color->SetSelection(i);
            }
        }
    }

    if (c_isInverse)
    {
        c_isInverse->SetValue(m_guiOptions->isInverse != 0);
    }

    if (c_undocumented)
    {
        c_undocumented->SetValue(m_options->use_undocumented);
    }

    if (c_monitor)
    {
        wxString hex_file(m_options->hex_file.c_str(), *wxConvCurrent);
        c_monitor->SetValue(hex_file);
    }

    if (c_htmlViewer)
    {
        wxString html_viewer(m_guiOptions->html_viewer.c_str(), *wxConvCurrent);
        c_htmlViewer->SetValue(html_viewer);
    }

    if (c_diskDir)
    {
        wxString disk_dir(m_options->disk_dir.c_str(), *wxConvCurrent);
        c_diskDir->SetValue(disk_dir);
    }

    for (x = 0; x <= 3; x++)
        if (c_drive[x])
        {
            wxString driveName(m_options->drive[x].c_str(), *wxConvCurrent);
            c_drive[x]->SetValue(driveName);
        }

    if (c_ramExtension)
    {
        c_ramExtension->SetSelection(m_options->isHiMem ? 1 : 0);
    }

    return wxWindow::TransferDataToWindow();
}

wxPanel *FlexemuOptionsDialog::CreateGuiOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxStaticText *pStatic;
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxGridSizer *pGridSizer = new wxGridSizer(2);

    pStatic = new wxStaticText(panel, -1, _("Screen geometry"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxTOP | wxLEFT, gap);
    c_geometry = new wxComboBox(panel, IDC_Geometry, wxT(""),
                                wxDefaultPosition, wxDefaultSize, 0, NULL,
                                wxCB_READONLY);
    pGridSizer->Add(c_geometry, 0, wxEXPAND);

    pStatic = new wxStaticText(panel, -1, _("Number of Colors"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxTOP | wxLEFT, gap);
    c_nColors = new wxComboBox(panel, IDC_nColors, wxT(""),
                               wxDefaultPosition, wxDefaultSize, 0, NULL,
                               wxCB_READONLY);
    pGridSizer->Add(c_nColors, 0, wxEXPAND);

    pStatic = new wxStaticText(panel, -1, _("Color"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxTOP | wxLEFT, gap);
    c_color = new wxComboBox(panel, IDC_Color, wxT(""), wxDefaultPosition,
                             wxDefaultSize, 0, NULL, wxCB_READONLY);
    pGridSizer->Add(c_color);
    pPanelSizer->Add(pGridSizer, 0, wxTOP, gap);

    c_isInverse = new wxCheckBox(panel, IDC_Inverse,
                                _("Display inverse colors"), wxDefaultPosition,
                                wxDefaultSize, 0);
    pPanelSizer->Add(c_isInverse, 0, wxTOP | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateMc6809OptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    c_undocumented = new wxCheckBox(panel, IDC_Inverse,
                                    _("Undocumented MC6809 instructions"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    pPanelSizer->Add(c_undocumented, 0, wxEXPAND | wxTOP | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateMemoryOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxString choices[2];

    choices[0] = _("2 x 96 KByte");
    choices[1] = _("2 x 288 KByte");
    c_ramExtension = new wxRadioBox(panel, IDC_RamExtension,
                                    _("RAM extension"), wxDefaultPosition,
                                    wxDefaultSize, WXSIZEOF(choices), choices,
                                    1, wxRA_SPECIFY_COLS);
    pPanelSizer->Add(c_ramExtension, 0, wxTOP | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreatePathOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pBoxSizer;
    wxStaticText *pStatic;
    wxButton *pButton;
    int i;
    wxString text;

    pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    pStatic = new wxStaticText(panel, -1, _("Disk/Monitor directory"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pBoxSizer->Add(pStatic, 0, wxLEFT | wxTOP, gap);
    c_diskDir = new wxTextCtrl(panel, IDC_DiskDir, wxT(""),
                               wxDefaultPosition, wxSize(textWidth, -1));
    pBoxSizer->Add(c_diskDir, 1, wxEXPAND);
    pButton = new wxButton(panel, IDC_DiskDirButton, _("..."),
                           wxDefaultPosition, wxSize(40, 25));
    pBoxSizer->Add(pButton, 0, wxTOP);
    pPanelSizer->Add(pBoxSizer, 0, wxTOP | wxEXPAND, gap);

    pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    pStatic = new wxStaticText(panel, -1, _("Monitor program"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pBoxSizer->Add(pStatic, 0, wxTOP | wxLEFT, gap);
    c_monitor = new wxTextCtrl(panel, IDC_Monitor, wxT(""), wxDefaultPosition,
                               wxSize(textWidth, -1));
    pBoxSizer->Add(c_monitor, 1, wxEXPAND);
    pButton = new wxButton(panel, IDC_MonitorButton, _("..."),
                           wxDefaultPosition, wxSize(40, 25));
    pBoxSizer->Add(pButton, 0, 0);
    pPanelSizer->Add(pBoxSizer, 0, wxEXPAND);

    for (i = 0; i < 4; ++i)
    {
        pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
        text.Printf(_("Disk for Drive %d"), i);
        pStatic = new wxStaticText(panel, -1, text,
                                   wxDefaultPosition, wxSize(stextWidth, -1));
        pBoxSizer->Add(pStatic, 0, wxTOP | wxLEFT, gap);
        c_drive[i] = new wxTextCtrl(panel, IDC_Drive0 + i, wxT(""),
                                    wxDefaultPosition, wxSize(textWidth, -1));
        pBoxSizer->Add(c_drive[i], 1, wxEXPAND);
        pButton = new wxButton(panel, IDC_Drive0Button + i, _("..."),
                               wxDefaultPosition, wxSize(40, 25));
        pBoxSizer->Add(pButton, 0, 0);
        pPanelSizer->Add(pBoxSizer, 0, wxEXPAND);
    }

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateDocuOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pBoxSizer;
    wxStaticText *pStatic;

    pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    pStatic = new wxStaticText(panel, -1, _("HTML Viewer"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pBoxSizer->Add(pStatic, 0, wxLEFT | wxTOP, gap);
    c_htmlViewer = new wxTextCtrl(panel, IDC_HTMLViewer, wxT(""),
                                  wxDefaultPosition, wxSize(textWidth, -1), 0);
    pBoxSizer->Add(c_htmlViewer, 1, wxEXPAND, gap);
    pStatic = new wxStaticText(panel, -1, wxT(""), wxDefaultPosition,
                               wxSize(40, 25));
    pBoxSizer->Add(pStatic, 0, 0); // dummy widget instead of a Button
    pPanelSizer->Add(pBoxSizer, 0, wxTOP | wxEXPAND, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateExpertOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText *pStatic;

    pStatic = new wxStaticText(panel, -1,
                               _("Expert options have appropriate default "
                                 "values. They should only be changed by "
                                 "experts."),
                               wxDefaultPosition, wxSize(-1, 50),
                               wxALIGN_LEFT);
    pPanelSizer->Add(pStatic, 0, wxEXPAND | wxTOP | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

void FlexemuOptionsDialog::OnInitDialog(wxInitDialogEvent &event)
{
    wxWindow *parent = this;
    wxTreebook *notebook;
    wxPanel *panel;
    wxButton *pButton;
    size_t pageId = 0;

    wxBoxSizer *pMainSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pButtonSizer = new wxBoxSizer(wxVERTICAL);

    notebook = new wxTreebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxBK_LEFT);

    pMainSizer->Add(notebook, 1, wxEXPAND | wxALL, gap);

    panel = CreateGuiOptionsPage(notebook);
    notebook->AddPage(panel, _("User Interface"), false);
    panel = CreateMemoryOptionsPage(notebook);
    notebook->AddPage(panel, _("Memory"), false);
    pageId++;
    panel = CreatePathOptionsPage(notebook);
    notebook->AddPage(panel, _("Files and Directories"), true);
    pageId++;
#ifndef WIN32
    panel = CreateDocuOptionsPage(notebook);
    notebook->AddPage(panel, _("Documentation"), false);
    pageId++;
#endif
    panel = CreateExpertOptionsPage(notebook);
    notebook->AddPage(panel, _("Expert Options"), false);
    pageId++;
    panel = CreateMc6809OptionsPage(notebook);
    notebook->AddSubPage(panel, _("MC6809"), false);

    pButton = new wxButton(parent, wxID_OK, _("&Ok"));
    pButtonSizer->Add(pButton, 0, wxTOP | wxRIGHT, gap);
    pButton = new wxButton(parent, wxID_CANCEL, _("&Cancel"));
    pButtonSizer->Add(pButton, 0, wxTOP | wxRIGHT, gap);
    pButton->SetFocus();

    pMainSizer->Add(pButtonSizer, 0, wxTOP, gap);

//    pMainSizer->Layout();

    SetSizer(pMainSizer);
    SetMinSize(wxSize(640,185));
    //SetAutoLayout(true);
    Layout();
    Centre(wxBOTH);

    wxDialog::OnInitDialog(event); // must be last command
}

/*
void FlexemuOptionsDialog::OnCloseWindow(wxCloseEvent &event)
{
    if (!event.CanVeto())
        Destroy();
    else
        wxDialog::OnCloseWindow(event);
}
*/
bool FlexemuOptionsDialog::Validate()
{
    // doing some verification of the values
    if (c_monitor != NULL && c_monitor->GetValue().IsEmpty())
    {
        wxMessageBox(_("Monitor program must not be empty"),
                     _("FSetup Error"), wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return false;
    }

    return wxWindow::Validate();
}

bool FlexemuOptionsDialog::TransferDataFromWindow(void)
{

    if (c_geometry)
    {
        wxString geometry;
        unsigned long x, y;

        geometry = c_geometry->GetValue();

        if (geometry.BeforeFirst(wxT('x')).ToULong(&x) &&
            geometry.AfterFirst(wxT('x')).ToULong(&y))
        {
            m_guiOptions->pixelSizeX = x / WINDOWWIDTH;
            m_guiOptions->pixelSizeY = y / WINDOWHEIGHT;
        }
    }

    if (c_nColors)
    {
        wxString nrOfColors;
        unsigned long n;

        nrOfColors = c_nColors->GetValue();

        if (nrOfColors.ToULong(&n))
        {
            m_guiOptions->nColors = n;
        }
    }

    if (c_color)
    {
        m_guiOptions->color =
            c_color->GetValue().mb_str(*wxConvCurrent);
        unsigned int i;
        wxString colorName;

        for (i = 0; i < WXSIZEOF(color_table); i++)
        {
            colorName = wxGetTranslation(color_table[i]);

            if (!colorName.Cmp(c_color->GetValue()))
            {
                wxString color(color_table[i]);
                m_guiOptions->color =
                    color.mb_str(*wxConvCurrent);
            }
        }
    };

    if (c_isInverse)
    {
        m_guiOptions->isInverse = c_isInverse->GetValue();
    };

    if (c_undocumented)
    {
        m_options->use_undocumented = (c_undocumented->GetValue() != 0);
    };

    if (c_monitor)
    {
        m_options->hex_file =
            c_monitor->GetValue().mb_str(*wxConvCurrent);
    };

#ifndef WIN32
    if (c_htmlViewer)
    {
        m_guiOptions->html_viewer =
            c_htmlViewer->GetValue().mb_str(*wxConvCurrent);
    };

#endif
    if (c_diskDir)
    {
        m_options->disk_dir =
            c_diskDir->GetValue().mb_str(*wxConvCurrent);
    };

    for (int i = 0; i <= 3; i++)
        if (c_drive[i])
            m_options->drive[i] =
                c_drive[i]->GetValue().mb_str(*wxConvCurrent);

    if (c_ramExtension)
    {
        m_options->isHiMem = c_ramExtension->GetSelection() > 0;
    };

    return wxWindow::TransferDataFromWindow();
}

wxString FlexemuOptionsDialog::OpenFilePrompter(
    const wxString &defautPath,
    const wxString &caption,
    const wxString &filter)
{
    wxString drive;

#ifdef WIN32
    char wd[PATH_MAX];

    getcwd((char *)wd, PATH_MAX);
#endif
    drive = wxFileSelector(
                caption,
                defautPath,
                wxT(""),
                filter,
                _("*.*"),
                wxFD_SAVE,
                this);
#ifdef WIN32
    chdir((char *)wd);
#endif
    return drive;
}

void FlexemuOptionsDialog::OnSelectDriveN(int n)
{
    wxString path;
    wxString diskDir;

    if (n > 3)
    {
        return;
    }

    if (c_diskDir)
    {
        diskDir = c_diskDir->GetValue();
    };

    path = OpenFilePrompter(diskDir, _("Select a Disk file"), _("*.DSK"));

    if (!diskDir.IsEmpty() &&
        (path.Find(diskDir) == 0) &&
        (path.Find(PATHSEPARATOR) >= 0))
    {
        path = path.Mid(diskDir.Len(), path.Len() - diskDir.Len());

        if (path.Find(PATHSEPARATOR) == 0)
        {
            path = path.Mid(1, path.length() - 1);
        }
    }

    if (!path.IsEmpty())
        if (c_drive[n])
        {
            c_drive[n]->SetValue(path);
        }
}

void FlexemuOptionsDialog::OnSelectDrive0(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDriveN(0);
}

void FlexemuOptionsDialog::OnSelectDrive1(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDriveN(1);
}

void FlexemuOptionsDialog::OnSelectDrive2(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDriveN(2);
}

void FlexemuOptionsDialog::OnSelectDrive3(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDriveN(3);
}

void FlexemuOptionsDialog::OnSelectDiskDir(wxCommandEvent &WXUNUSED(event))
{
    wxDirDialog *dialog;

    if (c_diskDir)
    {
        m_options->disk_dir =
            c_diskDir->GetValue().mb_str(*wxConvCurrent);
    };

    wxString disk_dir(m_options->disk_dir.c_str(), *wxConvCurrent);

    dialog = new wxDirDialog(this, _("Select folder with DSK files"),
                             disk_dir);

    if (dialog->ShowModal() == wxID_OK)
    {
        m_options->disk_dir = dialog->GetPath().mb_str(*wxConvCurrent);

        if (c_diskDir)
        {
            wxString disk_dir(m_options->disk_dir.c_str(), *wxConvCurrent);
            c_diskDir->SetValue(disk_dir);
        }
    }

    delete dialog;
}

void FlexemuOptionsDialog::OnSelectMonitor(wxCommandEvent &WXUNUSED(event))
{
    wxString path;
    wxString diskDir;

    if (c_diskDir)
    {
        diskDir = c_diskDir->GetValue();
    };

    path = OpenFilePrompter(diskDir, _("Select a monitor program"),
                            _("*.HEX"));

    if (!diskDir.IsEmpty() &&
        (path.Find(diskDir) == 0) &&
        (path.Find(PATHSEPARATOR) >= 0))
    {
        path = path.Mid(diskDir.Len(), path.Len() - diskDir.Len());

        if (path.Find(PATHSEPARATOR) == 0)
        {
            path = path.Mid(1, path.Len() - 1);
        }
    }

    if (!path.IsEmpty())
        if (c_monitor)
        {
            c_monitor->SetValue(path);
        }
}

