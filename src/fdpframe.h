/*
    fdpframe.h


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

#ifndef FDPFRAME_INCLUDED
#define FDPFRAME_INCLUDED

#include "misc1.h"
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    // Include your minimal set of headers here, or wx.h
    #include <wx/wx.h>
#endif

//#include <wx/mdi.h>
#include "bobserv.h"

class FileContainerIf;

/*------------------------------------------------------
 FlexParentFrame
 A MDI parent frame used to display FlexChildFrame's
--------------------------------------------------------*/
class FlexParentFrame: public wxMDIParentFrame, public BObserver
{
public:
    FlexParentFrame(wxWindow *parent, const wxWindowID id,
                    const wxString &title, const wxPoint &pos,
                    const wxSize &size, const long style);
    virtual ~FlexParentFrame();
    void OnChildFocus(wxChildFocusEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnNewContainer(wxCommandEvent &event);
    void OnOpenContainer(wxCommandEvent &event);
    void OnNewDirectory(wxCommandEvent &event);
    void OnOpenDirectory(wxCommandEvent &event);
    void OnOptions(wxCommandEvent &event);
    bool OpenContainer(const wxString &containerPath);
    void OpenChild(wxString &title, FileContainerIf *container);
    bool GetContainerProperties(int *tracks, int *sectors,
                                int *format, wxString &path);
    bool GetGlobalOptions(bool *autoTextFlag, wxString &viewer,
                          wxString &bootFile);
    void OnQuit(wxCommandEvent &event);
    void UpdateFrom(NotifyId id, void *param = nullptr) override;

private:
    void InitToolBar(wxToolBar *toolBar);

    DECLARE_EVENT_TABLE()
};

#endif

