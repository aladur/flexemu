/*
    fsetpdlg.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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
#include <wx/bmpcbox.h>
#include <wx/hyperlink.h>
#include <wx/filesys.h>

#include "misc1.h"
#include <string>
#include <memory>

#ifdef _MSC_VER
    #include <direct.h>
#endif

#include "fsetpdlg.h"
#include "fsetup.h"


int FlexemuOptionsDialog::ncolor_count[] = { 2, 8, 64 };

BEGIN_EVENT_TABLE(FlexemuOptionsDialog, wxDialog)
    EVT_BUTTON(IDC_Drive0Button, FlexemuOptionsDialog::OnSelectDrive0)
    EVT_BUTTON(IDC_Drive1Button, FlexemuOptionsDialog::OnSelectDrive1)
    EVT_BUTTON(IDC_Drive2Button, FlexemuOptionsDialog::OnSelectDrive2)
    EVT_BUTTON(IDC_Drive3Button, FlexemuOptionsDialog::OnSelectDrive3)
    EVT_BUTTON(IDC_MdcrDrive0Button, FlexemuOptionsDialog::OnSelectMdcrDrive0)
    EVT_BUTTON(IDC_MdcrDrive1Button, FlexemuOptionsDialog::OnSelectMdcrDrive1)
    EVT_BUTTON(IDC_DiskDirButton, FlexemuOptionsDialog::OnSelectDiskDir)
    EVT_BUTTON(IDC_MonitorButton, FlexemuOptionsDialog::OnSelectMonitor)
    EVT_INIT_DIALOG(FlexemuOptionsDialog::OnInitDialog)
    EVT_RADIOBOX(IDC_RamExtension, FlexemuOptionsDialog::OnRamExtensionChanged)
    EVT_RADIOBOX(IDC_EmulatedHardware,
            FlexemuOptionsDialog::OnEmulatedHardwareChanged)
    EVT_RADIOBOX(IDC_FrequencyChoices,
            FlexemuOptionsDialog::OnFrequencyChoicesChanged)
    EVT_CHECKBOX(IDC_MultiColorScheme,
            FlexemuOptionsDialog::OnMultiColorSchemeChanged)
    EVT_COMBOBOX(IDC_nColors,
            FlexemuOptionsDialog::OnNColorsChanged)
    //  EVT_CLOSE(FlexemuOptionsDialog::OnCloseWindow)
END_EVENT_TABLE()

const int FlexemuOptionsDialog::textWidth = 250;
const int FlexemuOptionsDialog::stextWidth = 150;
const int FlexemuOptionsDialog::gap = 5;

FlexemuOptionsDialog::FlexemuOptionsDialog(
    struct sGuiOptions &guiOptions,
    struct sOptions &options,
    wxWindow *parent,
    wxWindowID id,
    const wxString &title,
    const wxPoint &pos,
    const wxSize &size,
    long style,
    const wxString &name) :
    wxDialog(parent, id, title, pos, size, style, name),
    m_guiOptions(guiOptions), m_options(options),
    c_color(nullptr), c_isInverse(nullptr), c_undocumented(nullptr),
    c_geometry(nullptr), c_nColors(nullptr), c_monitor(nullptr),
    c_diskDir(nullptr),
    c_ramExtension(nullptr), c_flexibleMmu(nullptr),
    c_useRtc(nullptr), c_useRtcStatic(nullptr),
    c_emulatedHardware(nullptr),
    c_frequencyChoices(nullptr), c_frequency(nullptr),
    c_notebook(nullptr), c_multiColorScheme(nullptr)

{
    size_t i;

    for (i = 0; i < WXSIZEOF(c_drive); ++i)
    {
        c_drive[i] = nullptr;
    }

    for (i = 0; i < WXSIZEOF(c_mdcrDrive); ++i)
    {
        c_mdcrDrive[i] = nullptr;
    }
}

FlexemuOptionsDialog::~FlexemuOptionsDialog()
{
}

bool FlexemuOptionsDialog::TransferDataToWindow()
{
    bool hasSelection = false;
    wxString str;
    size_t i;
    int n = 0;

    for (int x = 1; x <= MAX_PIXELSIZEX; x++)
    {
        int y;

        for (y = 1; y <= MAX_PIXELSIZEY; y++)
        {
            str.Printf("%ix%i", WINDOWWIDTH * x, WINDOWHEIGHT * y);

            c_geometry->Append(str);

            if (m_guiOptions.pixelSizeX == x &&
                m_guiOptions.pixelSizeY == y)
            {
                c_geometry->SetSelection(n);
                hasSelection = true;
            }

            n++;
        }
    }
    if (!hasSelection)
    {
        c_geometry->SetSelection(0);
    }

    hasSelection = false;
    for (i = 0; i < WXSIZEOF(ncolor_count); i++)
    {
        str.Printf("%d", ncolor_count[i]);
        c_nColors->Append(str);

        if (m_guiOptions.nColors == ncolor_count[i])
        {
            c_nColors->SetSelection(i);
            hasSelection = true;
        }
    }
    if (!hasSelection)
    {
        c_nColors->SetSelection(0);
    }
    c_nColors->Enable(!m_options.isEurocom2V5);

    wxString colorName;
    DWord colorRGBValue;

    hasSelection = false;
    for (i = 0; i < color_count; i++)
    {
        colorName = wxGetTranslation(colors[i].colorName);
        getColorForName(colors[i].colorName, &colorRGBValue);
        c_color->Append(colorName,
                        CreateColorBitmap(wxColour(colorRGBValue),
                                          wxSize(16, 16)));

        if (!stricmp(m_guiOptions.color.c_str(), colors[i].colorName))
        {
            c_color->SetSelection(i);
            hasSelection = true;
        }
    }
    if (!hasSelection)
    {
        c_color->SetSelection(0);
    }

    bool isMultiColorSchemeEnabled =
            c_nColors->GetSelection() > 0 && !m_options.isEurocom2V5;
    bool isMultiColorSchemeChecked =
            isMultiColorSchemeEnabled &&
	    (0 == stricmp(m_guiOptions.color.c_str(), "default"));

    c_multiColorScheme->SetValue(isMultiColorSchemeChecked);
    c_multiColorScheme->Enable(isMultiColorSchemeEnabled);
    c_color->Enable(!isMultiColorSchemeChecked);

    c_isInverse->SetValue(m_guiOptions.isInverse != 0);

    c_undocumented->SetValue(m_options.use_undocumented);

    wxString hex_file(m_options.hex_file.c_str(), wxConvUTF8);
    c_monitor->SetValue(hex_file);

    wxString disk_dir(m_options.disk_dir.c_str(), wxConvUTF8);
    c_diskDir->SetValue(disk_dir);

    for (size_t x = 0; x < WXSIZEOF(c_drive); x++)
    {
        wxString driveName(m_options.drive[x].c_str(), wxConvUTF8);
        c_drive[x]->SetValue(driveName);
        c_drive[x]->Enable(!m_options.isEurocom2V5);
    }

    for (size_t x = 0; x < WXSIZEOF(c_mdcrDrive); x++)
    {
        wxString driveName(m_options.mdcrDrives[x].c_str(), wxConvUTF8);
        c_mdcrDrive[x]->SetValue(driveName);
        c_mdcrDrive[x]->Enable(m_options.isEurocom2V5);
    }

    int selection = 0;

    if (m_options.isRamExtension)
    {
        selection = 1;

        if (m_options.isHiMem)
        {
            selection = 2;
        }
    }

    c_ramExtension->SetSelection(selection);
    if (selection == 0)
    {
        c_nColors->SetSelection(0);
    }
    c_nColors->Enable(selection > 0);
    c_ramExtension->Enable(!m_options.isEurocom2V5);

    c_flexibleMmu->SetValue(m_options.isHiMem && m_options.isFlexibleMmu);
    c_flexibleMmu->Enable(m_options.isHiMem && !m_options.isEurocom2V5);

    c_useRtc->SetValue(m_options.useRtc);
    c_useRtc->Enable(!m_options.isEurocom2V5);
    c_useRtcStatic->Enable(!m_options.isEurocom2V5);

    c_emulatedHardware->SetSelection(m_options.isEurocom2V5 ? 0 : 1);

    if (m_options.frequency < 0.0f)
    {
        c_frequencyChoices->SetSelection(0);
        c_frequency->Enable(false);
        c_frequency->SetValue(std::to_string(ORIGINAL_FREQUENCY));
    }
    else
    if (m_options.frequency == 0.0f)
    {
        c_frequencyChoices->SetSelection(1);
        c_frequency->Enable(false);
    }
    else
    {
        c_frequencyChoices->SetSelection(2);
        c_frequency->SetValue(std::to_string(m_options.frequency));
    }

    return wxWindow::TransferDataToWindow();
}

wxPanel *FlexemuOptionsDialog::CreateEmulatedHardwareOptionsPage(
        wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pFrequencySizer;
    wxStaticText *pStatic;
    wxString hardwareChoices[2];
    wxString frequencyChoices[3];

    hardwareChoices[0] = _("Eurocom II V5 (Philips Mini DCR)");
    hardwareChoices[1] = _("Eurocom II V7 (Floppy Disk)");
    c_emulatedHardware = new wxRadioBox(panel, IDC_EmulatedHardware,
                                    _("Emulated Hardware"), wxDefaultPosition,
                                    wxDefaultSize,
                                    WXSIZEOF(hardwareChoices), hardwareChoices,
                                    1, wxRA_SPECIFY_COLS);
    pPanelSizer->Add(c_emulatedHardware, 0, wxLEFT, gap);
    int maxWidth = c_emulatedHardware->GetSize().GetWidth();
    frequencyChoices[0] = _("Use original frequency");
    frequencyChoices[1] = _("Emulate as fast as possible");
    frequencyChoices[2] = _("Set frequency:");
    c_frequencyChoices = new wxRadioBox(panel, IDC_FrequencyChoices,
                                    _("CPU Frequency"), wxDefaultPosition,
                                    wxSize(maxWidth, -1),
                                    WXSIZEOF(frequencyChoices),
                                    frequencyChoices,
                                    1, wxRA_SPECIFY_COLS);
    pPanelSizer->Add(c_frequencyChoices, 0, wxTOP | wxLEFT, gap);
    pFrequencySizer = new wxBoxSizer(wxHORIZONTAL);
    pPanelSizer->Add(pFrequencySizer, 0, wxTOP | wxLEFT, gap);
    pStatic = new wxStaticText(panel, -1, _("MHz"),
                               wxDefaultPosition, wxDefaultSize);
    int frequencyWidth = maxWidth - pStatic->GetSize().GetWidth() - gap;
    c_frequency = new wxTextCtrl(panel, IDC_Frequency, "",
                               wxDefaultPosition, wxSize(frequencyWidth, -1));
    pFrequencySizer->Add(c_frequency, 1);
    pFrequencySizer->Add(pStatic, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, gap);
    c_useRtc = new wxCheckBox(panel, IDC_UseRtc, _("MC146818 Realtime clock"),
                              wxDefaultPosition, wxSize(maxWidth, -1), 0);
    pPanelSizer->Add(c_useRtc, 0, wxTOP | wxLEFT, gap);
    c_useRtcStatic = new wxStaticText(panel, -1, _("(Hardware extension)"),
                                      wxDefaultPosition, wxDefaultSize);
    pPanelSizer->Add(c_useRtcStatic, 0, wxLEFT, 25);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateGuiOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxStaticText *pStatic;
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxGridSizer *pGridSizer = new wxGridSizer(2);

    pStatic = new wxStaticText(panel, -1, _("Screen geometry"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, gap);
    c_geometry = new wxComboBox(panel, IDC_Geometry, "",
                                wxDefaultPosition, wxDefaultSize, 0, nullptr,
                                wxCB_READONLY);
    pGridSizer->Add(c_geometry, 0, wxEXPAND);

    pStatic = new wxStaticText(panel, -1, _("Number of shades"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, gap);
    c_nColors = new wxComboBox(panel, IDC_nColors, "",
                               wxDefaultPosition, wxDefaultSize, 0, nullptr,
                               wxCB_READONLY);
    pGridSizer->Add(c_nColors, 0, wxEXPAND | wxTOP, 2);

    pStatic = new wxStaticText(panel, -1, _("Monochromatic color"),
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pGridSizer->Add(pStatic, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, gap);
    c_color = new wxBitmapComboBox(panel, IDC_Color, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
				   0, nullptr, wxCB_READONLY);
    pGridSizer->Add(c_color, 0, wxEXPAND | wxTOP, 2);
    pPanelSizer->Add(pGridSizer);

    c_multiColorScheme = new wxCheckBox(panel, IDC_MultiColorScheme,
                                _("Multi color scheme"),
				wxDefaultPosition, wxDefaultSize, 0);
    pPanelSizer->Add(c_multiColorScheme, 0, wxTOP | wxLEFT, gap);

    c_isInverse = new wxCheckBox(panel, IDC_Inverse,
                                _("Display inverse colors"), wxDefaultPosition,
                                wxDefaultSize, 0);
    pPanelSizer->Add(c_isInverse, 0, wxTOP | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateCpuOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    c_undocumented = new wxCheckBox(panel, IDC_Inverse,
                                    _("Undocumented MC6809 instructions"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    pPanelSizer->Add(c_undocumented, 0, wxEXPAND | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateHardwareOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    c_flexibleMmu = new wxCheckBox(panel, IDC_FlexibleMmu,
                                    _("More flexible MMU (Hardware modification)"),
                                    wxDefaultPosition, wxDefaultSize, 0);
    pPanelSizer->Add(c_flexibleMmu, 0, wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxPanel *FlexemuOptionsDialog::CreateMemoryOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxString choices[3];

    choices[0] = _("None");
    choices[1] = _("2 x 96 KByte");
    choices[2] = _("2 x 288 KByte");
    c_ramExtension = new wxRadioBox(panel, IDC_RamExtension,
                                    _("RAM extension"), wxDefaultPosition,
                                    wxDefaultSize, WXSIZEOF(choices), choices,
                                    1, wxRA_SPECIFY_COLS);
    pPanelSizer->Add(c_ramExtension, 0, wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

wxTextCtrl *FlexemuOptionsDialog::CreateFileControls(
        wxPanel *panel, wxBoxSizer *panelSizer, const wxString &text,
        int textId, int buttonId, int topBorderWidth)
{
    wxBoxSizer *pBoxSizer;
    wxStaticText *pStatic;
    wxButton *pButton;
    wxTextCtrl *textCtrl;

    pBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    pStatic = new wxStaticText(panel, -1, text,
                               wxDefaultPosition, wxSize(stextWidth, -1));
    pBoxSizer->Add(pStatic, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, gap);
    textCtrl = new wxTextCtrl(panel, textId, "",
                              wxDefaultPosition, wxSize(textWidth, -1));
    pBoxSizer->Add(textCtrl, 1, wxEXPAND);
    pButton = new wxButton(panel, buttonId, _("..."),
                           wxDefaultPosition, wxSize(40, 30));
    pBoxSizer->Add(pButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, gap);
    int flags = wxEXPAND | (topBorderWidth > 0 ? wxTOP : 0);
    panelSizer->Add(pBoxSizer, 0, flags, topBorderWidth);

    return textCtrl;
}

wxPanel *FlexemuOptionsDialog::CreatePathOptionsPage(wxBookCtrlBase *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxString text;
    uint32_t i;

    c_diskDir = CreateFileControls(panel, pPanelSizer, "Disk/Monitor directory",
                                   IDC_DiskDir, IDC_DiskDirButton, 0);

    c_monitor = CreateFileControls(panel, pPanelSizer, "Monitor program",
                                   IDC_Monitor, IDC_MonitorButton, 2);

    for (i = 0; i < WXSIZEOF(c_drive); ++i)
    {
        text.Printf(_("Disk Drive %d"), i);
        c_drive[i] = CreateFileControls(panel, pPanelSizer, text,
                                        IDC_Drive0 + i,
                                        IDC_Drive0Button + i, 2);
    }

    for (i = 0; i < WXSIZEOF(c_mdcrDrive); ++i)
    {
        text.Printf(_("Cassette Drive %d"), i);
        c_mdcrDrive[i] = CreateFileControls(panel, pPanelSizer, text,
                                            IDC_MdcrDrive0 + i,
                                            IDC_MdcrDrive0Button + i, 2);
    }

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
    pPanelSizer->Add(pStatic, 0, wxEXPAND | wxLEFT, gap);

    panel->SetSizer(pPanelSizer);

    return panel;
}

void FlexemuOptionsDialog::OnInitDialog(wxInitDialogEvent &event)
{
    wxWindow *parent = this;
    wxPanel *panel;
    wxButton *pButton;

    wxBoxSizer *pMainSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *pLeftSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *pRightSizer = new wxBoxSizer(wxVERTICAL);

    c_notebook = new wxTreebook(parent, wxID_ANY, wxDefaultPosition,
                                wxDefaultSize, wxBK_LEFT);

    auto *pEurocom2Link = CreateHyperlinkCtrl(parent,
                                          _("Eurocom II hardware description"),
                                          _("e2hwdesc.htm"));

    pLeftSizer->Add(pEurocom2Link, 0, wxTOP | wxLEFT, gap);
    pLeftSizer->Add(c_notebook, 0, wxALL, gap);
    pMainSizer->Add(pLeftSizer, 1);

    panel = CreateEmulatedHardwareOptionsPage(c_notebook);
    c_notebook->AddPage(panel, _("Emulated Hardware"), false);
    panel = CreateGuiOptionsPage(c_notebook);
    c_notebook->AddPage(panel, _("User Interface"), false);
    panel = CreateMemoryOptionsPage(c_notebook);
    c_notebook->AddPage(panel, _("Memory"), false);
    panel = CreatePathOptionsPage(c_notebook);
    c_notebook->AddPage(panel, _("Files and Directories"), true);
    panel = CreateExpertOptionsPage(c_notebook);
    c_notebook->AddPage(panel, _("Expert Options"), false);
    panel = CreateCpuOptionsPage(c_notebook);
    c_notebook->AddSubPage(panel, _("CPU"), false);
    panel = CreateHardwareOptionsPage(c_notebook);
    c_notebook->AddSubPage(panel, _("Hardware"), false);

    pButton = new wxButton(parent, wxID_OK, _("&Ok"));
    pRightSizer->Add(pButton, 0, wxTOP | wxRIGHT, gap);
    pButton = new wxButton(parent, wxID_CANCEL, _("&Cancel"));
    pRightSizer->Add(pButton, 0, wxTOP | wxRIGHT, gap);
    pButton->SetFocus();

    pMainSizer->Add(pRightSizer);

    SetSizer(pMainSizer);
    SetMinSize(wxSize(640, 330));

    Layout();
    Centre(wxBOTH);

    wxDialog::OnInitDialog(event); // must be last command
}

bool FlexemuOptionsDialog::Validate()
{
    // doing some verification of the values
    if (c_monitor->GetValue().IsEmpty())
    {
        c_notebook->SetSelection(3);
        c_monitor->SetFocus();
        c_monitor->SetSelection(-1, -1);
        wxMessageBox(_("Monitor program must not be empty"),
                     _("FSetup Error"), wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return false;
    }

    if (c_frequencyChoices->GetSelection() == 2)
    {
        std::string valueString(c_frequency->GetValue().ToUTF8().data());
        float value;

        try
        {
            value = std::stof(valueString);
        }
        catch (std::exception &)
        {
            c_notebook->SetSelection(0);
            c_frequency->SetFocus();
            c_frequency->SetSelection(-1, -1);
            wxMessageBox(_(
                       "CPU Frequency is not a valid floating point number."),
                       _("FSetup Error"), wxOK | wxCENTRE | wxICON_EXCLAMATION);
            return false;
        }

        if (value < 0.1)
        {
            c_notebook->SetSelection(0);
            c_frequency->SetFocus();
            c_frequency->SetSelection(-1, -1);
            wxMessageBox(_(
                       "CPU Frequency has a minimum of 0.1 MHz"),
                       _("FSetup Error"), wxOK | wxCENTRE | wxICON_EXCLAMATION);
            return false;
        }
    }

    return wxWindow::Validate();
}

bool FlexemuOptionsDialog::TransferDataFromWindow()
{
    size_t i;

    wxString geometry;
    unsigned long x = 0;
    unsigned long y = 0;

    geometry = c_geometry->GetValue();

    if (geometry.BeforeFirst('x').ToULong(&x) &&
        geometry.AfterFirst('x').ToULong(&y))
    {
        m_guiOptions.pixelSizeX = x / WINDOWWIDTH;
        m_guiOptions.pixelSizeY = y / WINDOWHEIGHT;
    }

    wxString nrOfColors;
    unsigned long n;

    nrOfColors = c_nColors->GetValue();

    if (nrOfColors.ToULong(&n))
    {
        m_guiOptions.nColors = n;
    }

    if (c_multiColorScheme->IsChecked() && c_nColors->GetSelection() > 0)
    {
         m_guiOptions.color = "default";
    }
    else
    {
        m_guiOptions.color = c_color->GetValue().ToUTF8().data();
        wxString colorName;

        for (i = 0; i < color_count; i++)
        {
            colorName = wxGetTranslation(colors[i].colorName);

            if (!colorName.Cmp(c_color->GetValue()))
            {
                m_guiOptions.color = colors[i].colorName;
            }
        }
    }

    m_guiOptions.isInverse = c_isInverse->GetValue();

    m_options.use_undocumented = (c_undocumented->GetValue() != 0);

    m_options.hex_file = c_monitor->GetValue().ToUTF8().data();

    m_options.disk_dir = c_diskDir->GetValue().ToUTF8().data();

    for (i = 0; i < WXSIZEOF(c_drive); i++)
    {
        m_options.drive[i] = c_drive[i]->GetValue().ToUTF8().data();
    }

    for (i = 0; i < WXSIZEOF(c_mdcrDrive); i++)
    {
        m_options.mdcrDrives[i] = c_mdcrDrive[i]->GetValue().ToUTF8().data();
    }

    m_options.isRamExtension = c_ramExtension->GetSelection() > 0;
    m_options.isHiMem = c_ramExtension->GetSelection() > 1;

    m_options.isFlexibleMmu =
            m_options.isHiMem & (c_flexibleMmu->GetValue() != 0);

    m_options.useRtc = (c_useRtc->GetValue() != 0);

    m_options.isEurocom2V5 = (c_emulatedHardware->GetSelection() == 0);

    switch (c_frequencyChoices->GetSelection())
    {
        case 0:
            m_options.frequency = -1.0f;
            break;

        case 1:
            m_options.frequency = 0.0f;
            break;

        case 2:
            std::string value = c_frequency->GetValue().ToUTF8().data();

            try
            {
                m_options.frequency = std::stof(value);
            }
            catch (std::exception &)
            {
                // This case should be prevented by Validate.
                m_options.frequency = -1.0f;
            }
            break;
    }

    if (!m_options.isRamExtension)
    {
        m_guiOptions.nColors = 2;
        m_options.isHiMem = false;
        m_options.isFlexibleMmu = false;
    }

    if (m_options.isRamExtension && !m_options.isHiMem)
    {
        m_options.isFlexibleMmu = false;
    }

    if (m_options.isEurocom2V5)
    {
        m_guiOptions.nColors = 2;
        m_options.useRtc = false;
        m_options.isRamExtension = false;
        m_options.isHiMem = false;
        m_options.isFlexibleMmu = false;
    }

    return wxWindow::TransferDataFromWindow();
}

wxString FlexemuOptionsDialog::OpenFilePrompter(
    const wxString &defautPath,
    const wxString &caption,
    const wxString &defaultExtension,
    const wxString &filter)
{
    wxString drive;

#ifdef _WIN32
    char wd[PATH_MAX];

    getcwd(wd, PATH_MAX);
#endif
    drive = wxFileSelector(
                caption,
                defautPath,
                "",
                defaultExtension,
                filter,
                wxFD_OPEN | wxFD_FILE_MUST_EXIST,
                this);
#ifdef _WIN32
    chdir(wd);
#endif
    return drive;
}

void FlexemuOptionsDialog::OnSelectDrive(wxTextCtrl &driveX, bool isDisk)
{
    wxString path;
    wxString diskDir;

    diskDir = c_diskDir->GetValue();

    if (isDisk)
    {
        path = OpenFilePrompter(
                diskDir, _("Select a Disk file"),"*.dsk",
            _("FLEX file containers (*.dsk;*.flx;*.wta)|*.dsk;*.flx;*.wta|"
                "All files (*.*)|*.*"));
    }
    else
    {
        path = OpenFilePrompter(
                diskDir, _("Select a MDCR file"), "*.mdcr",
            _("MDCR containers (*.mdcr)|*.mdcr|"
                "All files (*.*)|*.*"));
    }

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
    {
        driveX.SetValue(path);
    }
}

void FlexemuOptionsDialog::OnSelectDrive0(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_drive[0], true);
}

void FlexemuOptionsDialog::OnSelectDrive1(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_drive[1], true);
}

void FlexemuOptionsDialog::OnSelectDrive2(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_drive[2], true);
}

void FlexemuOptionsDialog::OnSelectDrive3(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_drive[3], true);
}

void FlexemuOptionsDialog::OnSelectMdcrDrive0(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_mdcrDrive[0], false);
}

void FlexemuOptionsDialog::OnSelectMdcrDrive1(wxCommandEvent &WXUNUSED(event))
{
    OnSelectDrive(*c_mdcrDrive[1], false);
}

void FlexemuOptionsDialog::OnSelectDiskDir(wxCommandEvent &WXUNUSED(event))
{
    std::unique_ptr<wxDirDialog> dialog;

    m_options.disk_dir = c_diskDir->GetValue().ToUTF8().data();

    wxString disk_dir(m_options.disk_dir.c_str(), wxConvUTF8);

    dialog = std::unique_ptr<wxDirDialog>(
            new wxDirDialog(this, _("Select folder with DSK files"), disk_dir));

    if (dialog->ShowModal() == wxID_OK)
    {
        m_options.disk_dir = dialog->GetPath().ToUTF8().data();

        c_diskDir->SetValue(dialog->GetPath());
    }
}

void FlexemuOptionsDialog::OnSelectMonitor(wxCommandEvent &WXUNUSED(event))
{
    wxString path;
    wxString diskDir;

    diskDir = c_diskDir->GetValue();

    path = OpenFilePrompter(diskDir, _("Select a monitor program"), "*.hex",
        _("Intel HEX files (*.hex)|*.hex|"
            "Motorola S-Record files (*.s19;*.srec;*.mot)|*.s19;*.srec;*.mot|"
            "FLEX binary files (*.cmd;*.bin)|*.cmd;*.bin|"
            "All files (*.*)|*.*"));

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
    {
        c_monitor->SetValue(path);
    }
}

void FlexemuOptionsDialog::OnRamExtensionChanged(
        wxCommandEvent &WXUNUSED(event))
{
    UpdateRamDependencies();
}

void FlexemuOptionsDialog::UpdateRamDependencies()
{
    c_ramExtension->Enable(c_emulatedHardware->GetSelection() != 0);
    c_flexibleMmu->Enable(c_ramExtension->IsEnabled() &&
                          c_ramExtension->GetSelection() > 1);

    UpdateColorDependencies();
}

void FlexemuOptionsDialog::OnEmulatedHardwareChanged(
        wxCommandEvent &WXUNUSED(event))
{
    UpdateHardwareDependencies();
}

void FlexemuOptionsDialog::UpdateHardwareDependencies()
{
    bool isEurocom2V7 = c_emulatedHardware->GetSelection() != 0;
    size_t x;

    c_useRtc->Enable(isEurocom2V7);
    c_useRtcStatic->Enable(isEurocom2V7);

    for (x = 0; x < WXSIZEOF(c_drive); x++)
    {
        c_drive[x]->Enable(isEurocom2V7);
    }

    for (x = 0; x < WXSIZEOF(c_mdcrDrive); x++)
    {
        c_mdcrDrive[x]->Enable(!isEurocom2V7);
    }

    UpdateRamDependencies();
}

void FlexemuOptionsDialog::OnFrequencyChoicesChanged(
        wxCommandEvent &WXUNUSED(event))
{
    bool isEnableFrequency = (c_frequencyChoices->GetSelection() == 2);

    c_frequency->Enable(isEnableFrequency);


    if (c_frequencyChoices->GetSelection() == 0)
    {
        c_frequency->SetValue(std::to_string(ORIGINAL_FREQUENCY));
    }
    else if (c_frequencyChoices->GetSelection() == 1)
    {
        c_frequency->SetValue("");
    }
}

void FlexemuOptionsDialog::OnMultiColorSchemeChanged(
        wxCommandEvent &WXUNUSED(event))
{
    UpdateColorDependencies();
}

void FlexemuOptionsDialog::OnNColorsChanged(
        wxCommandEvent &WXUNUSED(event))
{
    UpdateColorDependencies();
}

void FlexemuOptionsDialog::UpdateColorDependencies()
{
    c_nColors->Enable(c_ramExtension->IsEnabled() &&
                      c_ramExtension->GetSelection() != 0);

    if (c_nColors->IsEnabled())
    {
        c_multiColorScheme->Enable(c_nColors->GetSelection() != 0);
        c_color->Enable(!c_multiColorScheme->IsEnabled() || 
                        !c_multiColorScheme->IsChecked());
    }
    else
    {
        c_multiColorScheme->Enable(false);
        c_color->Enable(true);
    }
}

wxBitmap FlexemuOptionsDialog::CreateColorBitmap(const wxColour &color,
		                                 const wxSize &size)
{
    wxMemoryDC dc;
    wxBitmap bmp(size);
    dc.SelectObject(bmp);

    dc.SetBrush(wxBrush(color));
    dc.DrawRectangle(wxPoint(0, 0), size);
    dc.SelectObject(wxNullBitmap);

    return bmp;
}

wxGenericHyperlinkCtrl *FlexemuOptionsDialog::CreateHyperlinkCtrl(
                                     wxWindow *parent,
                                     const wxString &label,
                                     const wxString &htmlFile)
{
    wxString path = m_guiOptions.doc_dir + PATHSEPARATORSTRING + htmlFile;
    wxString url = wxFileSystem::FileNameToURL(path);

    return new wxGenericHyperlinkCtrl(parent, 0, label, url,
                                      wxDefaultPosition, wxDefaultSize,
                                      wxHL_DEFAULT_STYLE);
}

