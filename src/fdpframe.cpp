/*
    fdpframe.cpp


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


#ifdef _MSC_VER
#include "confignt.h"
#else
#include "config.h"
#endif
// define global variable with version string because
// wx-headers delete predefined VERSION
static char *_progVersion = VERSION;

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif

#include <misc1.h>

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
#include "fclipbrd.h"
#include "fcopyman.h"
#include "fmenufac.h"
#include "optdlg.h"

#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
#include "bitmaps/flexdisk.xpm"
#include "bitmaps/new_con.xpm"
#include "bitmaps/open_con.xpm"
#include "bitmaps/open_dir.xpm"
#endif

#define MAX_NR_OF_BITMAPS (11)

/*------------------------------------------------------
 FlexParentFrame implementation
--------------------------------------------------------*/

BEGIN_EVENT_TABLE(FlexParentFrame, wxMDIParentFrame)
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
	const wxString& title, const wxPoint& pos, const wxSize& size,
	const long style) : wxMDIParentFrame(parent, id, title, pos, size, style)
{
// give it an icon
#ifdef __WXMSW__
	SetIcon(wxIcon("AFLEXDISK_ICON"));
#else
	SetIcon(wxIcon( flexdisk_xpm ));
#endif
        wxMenuBar  *pMenuBar = new wxMenuBar( wxMB_DOCKABLE );

        pMenuBar->Append(FlexMenuFactory::CreateMenu(fFileMenuId),  "&File");
        pMenuBar->Append(FlexMenuFactory::CreateMenu(fExtrasMenuId),"&Extras");
        pMenuBar->Append(FlexMenuFactory::CreateMenu(fHelpMenuId),  "&Help");
	SetMenuBar(pMenuBar);

    // Accelerators
    wxAcceleratorEntry entries[9];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_CONTAINER);
    entries[1].Set(wxACCEL_CTRL, (int) 'O', MDI_OPEN_CONTAINER);
    entries[2].Set(wxACCEL_CTRL, (int) 'D', MDI_NEW_DIRECTORY);
    entries[3].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[4].Set(wxACCEL_CTRL, (int) 'A', MDI_SELECTALL);
    entries[5].Set(wxACCEL_CTRL, (int) 'C', MDI_COPY);
    entries[6].Set(wxACCEL_CTRL, (int) 'V', MDI_PASTE);
    entries[7].Set(wxACCEL_CTRL, (int) 'F', MDI_FIND);
    entries[8].Set(wxACCEL_NORMAL, WXK_DELETE, MDI_DELETE);
    wxAcceleratorTable accel(9, entries);
    SetAcceleratorTable(accel);

	// create a status bar
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
	wxStatusBar *statusBar = CreateStatusBar(2);
	const int fieldWidth[2] = { 120, -1 };
	statusBar->SetFieldsCount(2, fieldWidth);
#endif
	// create a tool bar
	CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE | wxTB_HORIZONTAL);
	InitToolBar(GetToolBar());
}

FlexParentFrame::~FlexParentFrame(void)
{
}

void FlexParentFrame::OnQuit(wxCommandEvent& WXUNUSED(event) )
{
	Close(TRUE);
}

void FlexParentFrame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap* bitmaps[3];

#ifdef __WXMSW__
	bitmaps[0] = new wxBitmap("new_con", wxBITMAP_TYPE_RESOURCE);
	bitmaps[1] = new wxBitmap("open_con", wxBITMAP_TYPE_RESOURCE);
	bitmaps[2] = new wxBitmap("open_dir", wxBITMAP_TYPE_RESOURCE);
#else
	bitmaps[0] = new wxBitmap( new_con_xpm );
	bitmaps[1] = new wxBitmap( open_con_xpm );
	bitmaps[2] = new wxBitmap( open_dir_xpm );
#endif

#ifdef __WXMSW__
	int width = 24;
#else
	int width = 16;
#endif
	int currentX = 5;

	toolBar->AddTool( MDI_NEW_CONTAINER, *bitmaps[0], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "New File Container");
	currentX += width + 5;
	toolBar->AddTool(MDI_OPEN_CONTAINER, *bitmaps[1], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Open File Container");
	currentX += width + 5;
	toolBar->AddTool(MDI_OPEN_DIRECTORY, *bitmaps[2], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Open Directory");
	currentX += width + 5;
	toolBar->Realize();

	for (int i = 0; i < 3; i++)
		delete bitmaps[i];
}

void FlexParentFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
	char *msgString;
	char *msgFormat =
"FLEXplorer V%s\n\
Copyright (C) W. Schwotzer  1998-2004\n\
FLEXplorer comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome\n\
to redistribute it under certain\n\
conditions. For more information\n\
look at file COPYING.\n\n\
http://www.geocities.com/flexemu/";

	msgString = new char[strlen(msgFormat)+strlen(_progVersion)+1];
	if (msgString) {
		sprintf(msgString, msgFormat, _progVersion);
      		(void)wxMessageBox(msgString, "About FLEXplorer");
	}
	delete [] msgString;
}

void FlexParentFrame::OnOpenContainer(wxCommandEvent& WXUNUSED(event) )
{
	FileContainerIf	*container;
	static wxString	containerPath;
	static wxString defaultDir;
	wxArrayString paths;
	wxString title;
	unsigned int i = 0;

	wxFileDialog dialog(
		this,
		"Select FLEX file containers",
		defaultDir,
		"",
		"DSK Container (*.dsk)|*.dsk|FLX Container (*.flx)|*.flx",
		wxOPEN | wxMULTIPLE);

	if (dialog.ShowModal() != wxID_OK)
		return;

	dialog.GetPaths(paths);

	for (i = 0; i < paths.GetCount(); ++i)
	{
		containerPath = paths.Item(i);
		defaultDir = containerPath.BeforeLast(PATHSEPARATOR);
	
		try {
			container = new FlexFileContainer(containerPath, "rb+");
		} catch (FlexException UNUSED(&e)) {
	   		try {
				container = new FlexFileContainer(containerPath, "rb");
	   		} catch (FlexException &e) {
				if (wxMessageBox(e.what(), "FLEXplorer Error",
				wxOK | wxCANCEL | wxCENTRE | wxICON_EXCLAMATION)
					 == wxCANCEL)
					return;
				continue;
	   		}
		}

#ifdef WIN32
		title = containerPath;
#endif
#ifdef UNIX
		title = containerPath.AfterLast(PATHSEPARATOR);
#endif
		if (container->IsWriteProtected())
			title += " [read-only]";
		OpenChild(title, container);
	}
}

void FlexParentFrame::OnNewContainer(wxCommandEvent& WXUNUSED(event) )
{
	FileContainerIf	*container;
	wxString	containerPath, directory, containerName;
	int		tracks, sectors, format;
	wxString title;

	containerPath = "";
	if (GetContainerProperties(&tracks, &sectors, &format, containerPath)) {
		directory = containerPath.BeforeLast(PATHSEPARATOR);
		containerName = containerPath.AfterLast(PATHSEPARATOR);
		try {
			container = FlexFileContainer::Create(directory,
                           containerName, tracks, sectors, format);
		} catch (FlexException &e) {
			wxMessageBox(e.what(), "FLEXplorer Error", wxOK | wxCENTRE | wxICON_EXCLAMATION);
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

void FlexParentFrame::OnOpenDirectory(wxCommandEvent& WXUNUSED(event) )
{
	FileContainerIf	*container;
	static wxString	containerPath;
	wxString	title;
	wxDirDialog	*dialog;

	dialog = new wxDirDialog(this, "Open a FLEX directory container",
			containerPath);
	if (dialog->ShowModal() == wxID_OK) {
		containerPath = dialog->GetPath();
#ifdef WIN32
		title = containerPath;
#endif
#ifdef UNIX
		title = containerPath.AfterLast(PATHSEPARATOR);
#endif
		container = new DirectoryContainer(containerPath);
		if (container->IsWriteProtected())
			title += " [read-only]";
		OpenChild(title, container);
	}
	delete dialog;
}

bool FlexParentFrame::GetContainerProperties(int *tracks, int *sectors, int *format, wxString &path)
{
	ContainerPropertiesDialog *dialog;
	wxPoint pos = GetPosition();

	pos.x += 10;
	pos.y += 10;
	
	dialog = new ContainerPropertiesDialog(this, pos, 80, 40, path);
	if (dialog->ShowModal() == wxID_OK)
	{
		*tracks = dialog->GetTracks();
		*sectors = dialog->GetSectors();
		path = dialog->GetPath();
		switch(dialog->GetFormat())
		{
			case 0:  *format = TYPE_DSK_CONTAINER; break;
			case 1:  *format = TYPE_FLX_CONTAINER; break;
			default: *format = TYPE_DSK_CONTAINER; break;
		}
		delete dialog;
		return true;
	}
	delete dialog;
	return false;
}

void FlexParentFrame::OnNewDirectory(wxCommandEvent& WXUNUSED(event) )
{
	FileContainerIf	*container;
	wxString		containerPath, directory, containerName;
	wxDirDialog		*dialog;

	dialog = new wxDirDialog(this,
		"Create a new FLEX directory container",
		"");
	if (dialog->ShowModal() == wxID_OK) {
		containerPath = dialog->GetPath();
		directory = containerPath.BeforeLast(PATHSEPARATOR);
		containerName = containerPath.AfterLast(PATHSEPARATOR);
		container = DirectoryContainer::Create(directory,
                               containerName, 80, 40);
		OpenChild(containerPath, container);
	}
	delete dialog;
}

void FlexParentFrame::OpenChild(const char *title, FileContainerIf *container)
{
	FlexChildFrame *childFrame;

	// create a child frame: the container view
	try {
		childFrame = new FlexChildFrame(
					this, title,
					wxPoint(-1, -1),
					wxSize(520, 500),
					wxDEFAULT_FRAME_STYLE,
					container);
	} catch (FlexException e) {
		wxMessageBox(e.what(), "FLEXplorer Error",
			wxOK | wxCENTRE | wxICON_EXCLAMATION);
		return;
	}
	childFrame->Attach(&SFlexFileClipboard::Instance());
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__)
	// statusbar support with GTK, X11, MOTIF
	childFrame->GetListControl()->Attach(this);
#endif
	childFrame->Show(TRUE);
}

void FlexParentFrame::OnSize(wxSizeEvent& WXUNUSED(event) )
{
    int w, h;
    GetClientSize(&w, &h);
    GetClientWindow()->SetSize(0, 0, w, h);
}

void FlexParentFrame::Update(const void *pId)
{
	int id = *(int *)pId;
	const FlexDiskListCtrl	*listCtrl;
	wxStatusBar				*sBar;
	if (!GetActiveChild())
		return;
	listCtrl = ((FlexChildFrame *)GetActiveChild())->GetListControl();
	if (listCtrl && id == OBSERVE_STATUS_BAR && (sBar = GetStatusBar())) {
		char buf[50];

		sprintf(buf, "%d File(s) selected", listCtrl->GetFileCount());
		sBar->SetStatusText(buf, 0);
		sprintf(buf, "%d Byte", listCtrl->GetTotalSize());
		sBar->SetStatusText(buf, 1);
	}
}

void FlexParentFrame::OnOptions(wxCommandEvent& WXUNUSED(event) )
{
	wxString viewer;
	wxString bootFile;
	bool autoTextFlag;

	viewer = FlexDiskListCtrl::fileViewer;
	bootFile = (const char *)FlexFileContainer::bootSectorFile;
	autoTextFlag = FlexCopyManager::autoTextConversion;
	if (GetGlobalOptions(&autoTextFlag, viewer, bootFile)) {
		FlexCopyManager::autoTextConversion = autoTextFlag;
		FlexDiskListCtrl::fileViewer = viewer;
		FlexFileContainer::bootSectorFile = (const char *)bootFile;
	}
}

bool FlexParentFrame::GetGlobalOptions(bool *autoTextFlag,
	wxString &viewer,
	wxString &bootFile)
{
	GlobalOptionsDialog *dialog;
	wxPoint pos = GetPosition();

	pos.x += 10;
	pos.y += 10;
	dialog = new GlobalOptionsDialog(this, pos,
		*autoTextFlag, bootFile, viewer);
	if (dialog->ShowModal() == wxID_OK) {
		*autoTextFlag = dialog->GetAutoTextFlag();
		bootFile = dialog->GetBootSectorFile();
		viewer = dialog->GetViewer();
		delete dialog;
		return true;
	}
	delete dialog;
	return false;
}

