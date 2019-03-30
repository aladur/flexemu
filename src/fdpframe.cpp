/*
    fdpframe.cpp


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


#ifdef _MSC_VER
    #include "confignt.h"
#else
    #include "config.h"
#endif
// define global variable with version string because
// wx-headers delete predefined VERSION
static const char *_progVersion = VERSION;

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>

#include "misc1.h"

#ifdef WIN32
    #ifdef _MSC_VER
        #include <direct.h>
    #endif
#endif

#include "fdpframe.h"
#include "fdcframe.h"
#include "flexdisk.h"
#include "ffilecnt.h"
#include "dircont.h"
#include "fdirent.h"
#include "fcinfo.h"
#include "contpdlg.h"
#include "fcopyman.h"
#include "fmenufac.h"
#include "optdlg.h"
#include "mdcrtape.h"
#include "fdfdnd.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    #include "bitmaps/flexdisk.xpm"
    #include "bitmaps/flexdisk64.xpm"
    #include "bitmaps/new_con.xpm"
    #include "bitmaps/open_con.xpm"
    #include "bitmaps/open_dir.xpm"
#endif

#define MAX_NR_OF_BITMAPS (11)


/*------------------------------------------------------
 FlexParentFrame implementation
--------------------------------------------------------*/

BEGIN_EVENT_TABLE(FlexParentFrame, wxMDIParentFrame)
    EVT_CHILD_FOCUS(FlexParentFrame::OnChildFocus)
    EVT_MENU(MDI_ABOUT, FlexParentFrame::OnAbout)
    EVT_MENU(MDI_NEW_CONTAINER, FlexParentFrame::OnNewContainer)
    EVT_MENU(MDI_OPEN_CONTAINER, FlexParentFrame::OnOpenContainer)
    EVT_MENU(MDI_NEW_DIRECTORY, FlexParentFrame::OnNewDirectory)
    EVT_MENU(MDI_OPEN_DIRECTORY, FlexParentFrame::OnOpenDirectory)
    EVT_MENU(MDI_OPTIONS, FlexParentFrame::OnOptions)
    EVT_MENU(MDI_QUIT, FlexParentFrame::OnQuit)
    EVT_SIZE(FlexParentFrame::OnSize)
END_EVENT_TABLE()

FlexParentFrame::FlexParentFrame(wxWindow *parent, const wxWindowID id,
                                 const wxString &title, const wxPoint &pos,
                                 const wxSize &size, const long style)
    : wxMDIParentFrame(parent, id, title, pos, size, style)
{
    // give it an icon
#ifdef __WXMSW__
    SetIcon(wxIcon("AFLEXDISK_ICON"));
#else
    SetIcon(wxIcon(flexdisk_xpm));
#endif
    wxMenuBar  *pMenuBar = new wxMenuBar(wxMB_DOCKABLE);

    pMenuBar->Append(FlexMenuFactory::CreateFileMenu(),  _("&File"));
    pMenuBar->Append(FlexMenuFactory::CreateExtrasMenu(), _("&Extras"));
    pMenuBar->Append(FlexMenuFactory::CreateHelpMenu(),  _("&Help"));
    SetMenuBar(pMenuBar);

    // Accelerators
    size_t i = 0;
    wxAcceleratorEntry entries[9];

    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('N'), MDI_NEW_CONTAINER);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('O'), MDI_OPEN_CONTAINER);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('D'), MDI_NEW_DIRECTORY);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('X'), MDI_QUIT);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('A'), MDI_SELECTALL);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('C'), MDI_COPY);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('V'), MDI_PASTE);
    entries[i++].Set(wxACCEL_CTRL, static_cast<int>('F'), MDI_FIND);
    entries[i++].Set(wxACCEL_NORMAL, WXK_DELETE, MDI_DELETE);
    wxAcceleratorTable accel(WXSIZEOF(entries), entries);
    SetAcceleratorTable(accel);

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    // create a status bar
    wxStatusBar *statusBar = CreateStatusBar(3);
    const int fieldWidth[3] = { 300, -1, -1 };
    statusBar->SetFieldsCount(3, fieldWidth);
#endif

    // create a tool bar
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());

#ifdef wxUSE_DRAG_AND_DROP
#ifndef __WXMOTIF__
    SetDropTarget(new FileDropTarget(this));
#endif
#endif
}

FlexParentFrame::~FlexParentFrame()
{
}

void FlexParentFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
    Close(TRUE);
}

void FlexParentFrame::InitToolBar(wxToolBar *toolBar)
{
#ifdef __WXMSW__
    auto newContainerBitmap = wxBitmap("new_con",wxBITMAP_TYPE_RESOURCE);
    auto openContainerBitmap = wxBitmap("open_con",wxBITMAP_TYPE_RESOURCE);
    auto openDirectoryBitmap = wxBitmap("open_dir",wxBITMAP_TYPE_RESOURCE);
#else
    auto newContainerBitmap = wxBitmap(new_con_xpm);
    auto openContainerBitmap = wxBitmap(open_con_xpm);
    auto openDirectoryBitmap = wxBitmap(open_dir_xpm);
#endif

#ifdef __WXMSW__
    int width = 24;
#else
    int width = 16;
#endif
    int currentX = 5;
    wxSize size(32, 32);

    toolBar->SetToolBitmapSize(size);
    toolBar->AddTool(MDI_NEW_CONTAINER, _("New File Container"),
                     newContainerBitmap, wxNullBitmap);
    toolBar->SetToolLongHelp(MDI_NEW_CONTAINER, "Create a new File Container");

    currentX += width + 5;
    toolBar->AddTool(MDI_OPEN_CONTAINER, _("Open File Container"),
                     openContainerBitmap, wxNullBitmap);
    toolBar->SetToolLongHelp(MDI_OPEN_CONTAINER,
                             "Open an existing File Container");

    currentX += width + 5;
    toolBar->AddTool(MDI_OPEN_DIRECTORY, _("Open Directory"),
                     openDirectoryBitmap, wxNullBitmap);
    toolBar->SetToolLongHelp(MDI_OPEN_DIRECTORY,
                             "Open an existing Directory as File Container");

    toolBar->Realize();
}

void FlexParentFrame::OnChildFocus(wxChildFocusEvent &event)
{
    wxWindow *pWindow = event.GetWindow();


    if (pWindow == GetActiveChild())
    {
        UpdateFrom(NotifyId::UpdateStatusBar);
    }
}

void FlexParentFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxString msgFormat;
    wxString progVersion(wxString("V") + _progVersion);
    wxAboutDialogInfo aboutInfo;

    msgFormat += _("Explore FLEX disk and directory containers.\n");
    msgFormat += _("\n");
    msgFormat += _("FLEXplorer comes with ABSOLUTELY NO WARRANTY.\n");
    msgFormat += _("This is free software, and You are welcome\n");
    msgFormat += _("to redistribute it under certain conditions.\n");
    msgFormat += _("\n");
    msgFormat += _("Have Fun!\n");

    aboutInfo.SetName("FLEXplorer");
    aboutInfo.SetVersion(progVersion);
    aboutInfo.SetCopyright("Copyright (C) 1998-2019\n");
    aboutInfo.AddDeveloper("Wolfgang Schwotzer");
    aboutInfo.SetWebSite("https://aladur.neocities.org/flexemu");
    aboutInfo.SetDescription(msgFormat);

#ifdef __WXMSW__
    aboutInfo.SetIcon(wxIcon("AFLEXDISK_ICON"));
#else
    aboutInfo.SetIcon(wxIcon(flexdisk64_xpm));
#endif

    wxAboutBox(aboutInfo);
}

void FlexParentFrame::OnOpenContainer(wxCommandEvent &WXUNUSED(event))
{
    static wxString defaultDir;
    wxArrayString paths;
    unsigned int i = 0;

    wxFileDialog dialog(
        this,
        _("Select FLEX file containers"),
        defaultDir,
        "",
        _("FLEX file containers (*.dsk;*.flx;*.wta)|*.dsk;*.flx;*.wta|"
            "All files (*.*)|*.*"),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    dialog.GetPaths(paths);

    for (i = 0; i < paths.GetCount(); ++i)
    {
        defaultDir = paths.Item(i).BeforeLast(PATHSEPARATOR);
        if (!OpenContainer(paths.Item(i)))
        {
            break;
        }
    }
}

bool FlexParentFrame::OpenContainer(const wxString &wxContainerPath)
{
    FileContainerIf *container;
    auto containerPath = std::string(wxContainerPath.mb_str());

    try
    {
        struct stat sbuf;

        if (!stat(containerPath.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode))
        {
            if (endsWithPathSeparator(containerPath))
            {
                containerPath = containerPath.substr(0, containerPath.size()-1);
            }

            container = new DirectoryContainer(containerPath.c_str());
        }
        else
        {
            try
            {
                container = new FlexFileContainer(containerPath.c_str(), "rb+");
            }
            catch (FlexException &)
            {
                // 2nd try opening read-only.
                container = new FlexFileContainer(containerPath.c_str(), "rb");
            }
        }
    }
    catch (FlexException &ex)
    {
        printf("Exception\n");
        int result = wxMessageBox(ex.what(),
                             _("FLEXplorer Error"), wxOK |
                             wxCANCEL | wxCENTRE | wxICON_EXCLAMATION);

        return (result != wxCANCEL);
    }

#ifdef WIN32
    auto title = wxContainerPath;
#endif
#ifdef UNIX
    auto title = wxString(getFileName(containerPath).c_str(), wxConvUTF8);
#endif

    if (container->IsWriteProtected())
    {
        title += _(" [read-only]");
    }

    OpenChild(title, container);

    return true;
}

void FlexParentFrame::OnNewContainer(wxCommandEvent &WXUNUSED(event))
{
    wxString    containerPath, directory, containerName;
    int     tracks, sectors, format;
    wxString title;

    containerPath = "";

    if (GetContainerProperties(&tracks, &sectors, &format, containerPath))
    {
        FileContainerIf *container;

        directory = containerPath.BeforeLast(PATHSEPARATOR);
        containerName = containerPath.AfterLast(PATHSEPARATOR);

        try
        {
            if (format == TYPE_MDCR_CONTAINER)
            {
                MiniDcrTapePtr mdcr = MiniDcrTape::Create(
                            containerPath.ToUTF8().data());
                // DCR containers can be created but not displayed in
                // FLEXplorer, so immediately return.
                return;
            }

            container = FlexFileContainer::Create(
                            directory.ToUTF8().data(),
                            containerName.ToUTF8().data(),
                            tracks, sectors, format);
        }
        catch (FlexException &ex)
        {
            wxMessageBox(ex.what(), _("FLEXplorer Error"),
                         wxOK | wxCENTRE | wxICON_EXCLAMATION);
            return;
        }

#ifdef WIN32
        title = containerPath;
#endif
#ifdef UNIX
        title = containerPath.AfterLast(PATHSEPARATOR);
#endif
        OpenChild(title, container);
    }
}

void FlexParentFrame::OnOpenDirectory(wxCommandEvent &WXUNUSED(event))
{
    static wxString containerPath;

    auto dialog = std::unique_ptr<wxDirDialog>(
                 new wxDirDialog(this, _("Open a FLEX directory container"),
                             containerPath));

    if (dialog->ShowModal() == wxID_OK)
    {
        containerPath = dialog->GetPath();

        OpenContainer(dialog->GetPath());
    }
}

bool FlexParentFrame::GetContainerProperties(int *tracks, int *sectors,
        int *format, wxString &path)
{
    wxPoint pos = GetPosition();

    pos.x += 10;
    pos.y += 10;

    auto dialog = std::unique_ptr<ContainerPropertiesDialog>(
                 new ContainerPropertiesDialog(this, pos, 80, 40, path));

    if (dialog->ShowModal() == wxID_OK)
    {
        *tracks = dialog->GetTracks();
        *sectors = dialog->GetSectors();
        path = dialog->GetPath();

        switch (dialog->GetFormat())
        {
            case 0:
                *format = TYPE_DSK_CONTAINER;
                break;

            case 1:
                *format = TYPE_FLX_CONTAINER;
                break;

            case 2:
                *format = TYPE_MDCR_CONTAINER;
                break;

            default:
                *format = TYPE_DSK_CONTAINER;
                break;
        }

        return true;
    }

    return false;
}

void FlexParentFrame::OnNewDirectory(wxCommandEvent &WXUNUSED(event))
{
    wxString        containerPath, directory, containerName;

    auto dialog = std::unique_ptr<wxDirDialog>(new wxDirDialog(this,
                             _("Create a new FLEX directory container"),
                             ""));

    if (dialog->ShowModal() == wxID_OK)
    {
        FileContainerIf *container;

        containerPath = dialog->GetPath();
        directory = containerPath.BeforeLast(PATHSEPARATOR);
        containerName = containerPath.AfterLast(PATHSEPARATOR);
        container = DirectoryContainer::Create(
                        directory.ToUTF8().data(),
                        containerName.ToUTF8().data(),
                        80, 40);
        OpenChild(containerPath, container);
    }
}

void FlexParentFrame::OpenChild(wxString &title, FileContainerIf *container)
{
    FlexChildFrame *childFrame;

    // create a child frame: the container view
    try
    {
        childFrame = new FlexChildFrame(
            this, title,
            wxPoint(-1, -1),
            wxSize(520, 500),
            wxDEFAULT_FRAME_STYLE,
            container);
    }
    catch (FlexException &ex)
    {
        wxMessageBox(ex.what(), _("FLEXplorer Error"),
                     wxOK | wxCENTRE | wxICON_EXCLAMATION);
        return;
    }

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
    // statusbar support with GTK, X11, MOTIF
    childFrame->GetListControl().Attach(*this);
#endif
    childFrame->Show(TRUE);
}

void FlexParentFrame::OnSize(wxSizeEvent &WXUNUSED(event))
{
    int w, h;
    GetClientSize(&w, &h);
    GetClientWindow()->SetSize(0, 0, w, h);
}

void FlexParentFrame::UpdateFrom(NotifyId id, void *)
{
    if (id == NotifyId::UpdateStatusBar)
    {
        if (!GetActiveChild())
        {
            return;
        }

        const auto &listCtrl =
            ((FlexChildFrame *)GetActiveChild())->GetListControl();

        wxStatusBar *statusBar = GetStatusBar();
        if (statusBar)
        {
            wxString str;

            str.Printf(_("%d File(s) selected"), listCtrl.GetFileCount());
            statusBar->SetStatusText(str, 1);
            str.Printf(_("%d Byte"), listCtrl.GetTotalSize());
            statusBar->SetStatusText(str, 2);
        }
    }
}

void FlexParentFrame::OnOptions(wxCommandEvent &WXUNUSED(event))
{
    bool autoTextFlag;

    wxString bootFile(FlexFileContainer::bootSectorFile.c_str(), wxConvUTF8);
    autoTextFlag = FlexCopyManager::autoTextConversion;

    if (GetGlobalOptions(&autoTextFlag, bootFile))
    {
        FlexCopyManager::autoTextConversion = autoTextFlag;
        FlexFileContainer::bootSectorFile = bootFile.ToUTF8().data();
    }
}

bool FlexParentFrame::GetGlobalOptions(bool *autoTextFlag, wxString &bootFile)
{
    wxPoint pos = GetPosition();

    pos.x += 10;
    pos.y += 10;
    auto dialog = std::unique_ptr<GlobalOptionsDialog>(
                 new GlobalOptionsDialog(this, pos, *autoTextFlag, bootFile));

    if (dialog->ShowModal() == wxID_OK)
    {
        *autoTextFlag = dialog->GetAutoTextFlag();
        bootFile = dialog->GetBootSectorFile();
        return true;
    }

    return false;
}

