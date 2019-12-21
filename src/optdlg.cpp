/*
    optdlg.cpp


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
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/dialog.h"
#include "wx/valgen.h"
#include "warnon.h"

#include "misc1.h"

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "optdlg.h"


#include "warnoff.h"
BEGIN_EVENT_TABLE(GlobalOptionsDialog, wxDialog)
    EVT_BUTTON(IDC_BootSectorFileButton,
               GlobalOptionsDialog::OnSelectBootSectorFile)
    //  EVT_BUTTON(wxID_OK,                  GlobalOptionsDialog::OnOK)
    //  EVT_BUTTON(wxID_CANCEL,              GlobalOptionsDialog::OnCancel)
END_EVENT_TABLE()
#include "warnon.h"

GlobalOptionsDialog::GlobalOptionsDialog(wxWindow *parent,
        const wxPoint &pos /* = wxDefaultPosition */,
        const bool autoTextFlag /* = false */,
        wxString bootFile /* = BOOT_FILE */) :
    wxDialog(parent, 111, _("Global Options"), pos),
    m_bootSectorFile(bootFile),
    m_autoTextFlag(autoTextFlag),
    c_bootSectorFile(nullptr), c_autoTextFlag(nullptr)
{
    wxButton     *pButton;
    wxStaticText *pStatic;

    wxBoxSizer *pMainSizer   = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pWidgetSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pFileSizer   = new wxBoxSizer(wxHORIZONTAL);

    c_autoTextFlag = new wxCheckBox(this, IDC_AutoTextFlag,
                                    _("Automatic Text Conversion"),
                                    wxDefaultPosition, wxDefaultSize, 0,
                                    wxGenericValidator(&m_autoTextFlag));
    pWidgetSizer->Add(c_autoTextFlag, 0, wxALL, 5);

    pStatic = new wxStaticText(this, -1, _("Boot Sector file"),
                               wxDefaultPosition, wxSize(100, -1));
    pFileSizer->Add(pStatic, 0, wxALL, 5);
    c_bootSectorFile = new wxTextCtrl(this, IDC_BootSectorFile, "",
                                      wxDefaultPosition, wxSize(200, -1), 0,
                                      wxTextValidator(wxFILTER_NONE,
                                              &m_bootSectorFile));
    pFileSizer->Add(c_bootSectorFile, 0, wxALL, 5);
    pButton = new wxButton(this, IDC_BootSectorFileButton, _("..."),
                           wxDefaultPosition, wxSize(40, -1));
    pFileSizer->Add(pButton, 0, wxALL, 5);
    pWidgetSizer->Add(pFileSizer);

    pMainSizer->Add(pWidgetSizer);

    pButton = new wxButton(this, wxID_OK, _("&Ok"));
    pButtonSizer->Add(pButton, 0, wxALL, 5);
    pButton = new wxButton(this, wxID_CANCEL, _("&Cancel"));
    pButtonSizer->Add(pButton, 0, wxALL, 5);

    pMainSizer->Add(pButtonSizer);

    SetSizer(pMainSizer);
    pMainSizer->SetSizeHints(this);
}

GlobalOptionsDialog::~GlobalOptionsDialog()
{
}

void GlobalOptionsDialog::OnSelectBootSectorFile(wxCommandEvent &WXUNUSED(
            event))
{
    wxString bootSectorFile;
#ifdef WIN32
    char  wd[PATH_MAX];

    getcwd(wd, PATH_MAX);
#endif
    bootSectorFile = wxFileSelector(
                         _("Select a Boot Sector file to be used"),
                         "",
                         "",
                         _("*"),
                         _("*"),
                         wxFD_SAVE,
                         this);
#ifdef WIN32
    chdir(wd);
#endif

    if (!bootSectorFile.IsEmpty())
    {
        m_bootSectorFile = bootSectorFile;

        if (c_bootSectorFile)
        {
            c_bootSectorFile->SetValue(m_bootSectorFile);
        }
    }
}

