/*
    fdlist.h


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

#ifndef FDLIST_INCLUDED
#define FDLIST_INCLUDED

#include "misc1.h"

#include <wx/textctrl.h>
#include <wx/listctrl.h>

#include "flexdisk.h"
#include "fddnd.h"
#include "flexerr.h"

#define LIST_CTRL   1000
#define OBSERVE_STATUS_BAR      (545)

class BObserver;
class FileContainerIf;
class FlexDirEntry;
class FlexFileBuffer;
class FlexFileList;

/*------------------------------------------------------
 FlexDiskListCtrl
 A specialized ListCtrl used to display a FLEX Container
--------------------------------------------------------*/

class FlexDiskListCtrl : public wxListCtrl
{
public:
    FlexDiskListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                     const wxSize &size, long style,
                     FileContainerIf *container,
                     const wxValidator &validator = wxDefaultValidator,
                     const wxString &name = wxT("flexDiskListCtrl"));
    virtual ~FlexDiskListCtrl();

    const wxMenu *GetMenu();
    inline void  Attach(BObserver *anObserver) const
    {
        m_statusbarObserver = anObserver;
    };
    inline void  Detach(BObserver *anObserver) const
    {
        if (m_statusbarObserver == anObserver)
        {
            m_statusbarObserver = nullptr;
        }
    };
    inline int GetTotalSize() const
    {
        return m_totalSize;
    };
    inline int GetFileCount() const
    {
        return GetSelectedItemCount();
    };
    inline FileContainerIf *GetContainer() const
    {
        return m_container;
    };
    static wxString GetFileDescription(const FlexDirEntry *pDe);

#ifdef wxUSE_DRAG_AND_DROP
    bool PasteFrom(FlexDnDFiles &files);
#endif
    int UpdateItems();
    void UpdateItem(int item, FlexDirEntry &de);
    DECLARE_MENUCOMMAND(OnSelectAll)
    DECLARE_MENUCOMMAND(OnDeselectAll)
    DECLARE_MENUCOMMAND(OnFind)
    DECLARE_MENUCOMMAND(OnCopy)
    DECLARE_MENUCOMMAND(OnPaste)
    DECLARE_MENUCOMMAND(OnDelete)
    DECLARE_MENUCOMMAND(OnRename)
    DECLARE_MENUCOMMAND(OnView)
    DECLARE_MENUCOMMAND(OnSetWriteProtect)
    DECLARE_MENUCOMMAND(OnClearWriteProtect)
    DECLARE_MENUCOMMAND(OnSetReadProtect)
    DECLARE_MENUCOMMAND(OnClearReadProtect)
    DECLARE_MENUCOMMAND(OnSetDeleteProtect)
    DECLARE_MENUCOMMAND(OnClearDeleteProtect)
    DECLARE_MENUCOMMAND(OnSetCatalogProtect)
    DECLARE_MENUCOMMAND(OnClearCatalogProtect)
    DECLARE_MENUCOMMAND(OnViewProperties)

    static wxString fileViewer;

private:
    void Notify();
    int GetSelections(long **pItems) const;
    void DeleteSelectedItems(bool askUser = TRUE);
    void RenameSelectedItems();
    void ViewSelectedItems();
    void SetPropertyOnSelectedItems(int protection, bool isToBeSet);
    void GetFileList(FlexFileList &fileList);
    void CopyToClipboard();
    bool PasteFromClipboard();
    void DeselectAllFiles();
    void SelectAllFiles();
    void FindFiles();
    void BeforeDeleteAllItems();
    /*
        void OnGetInfo(wxListEvent& event);
        void OnSetInfo(wxListEvent& event);
    */
#ifndef __WXMOTIF__
    void OnBeginDrag(wxListEvent &event);
#endif
    void OnRightMouseDown(wxMouseEvent &event);
    void OnLeftMouseDClick(wxMouseEvent &event);
    void OnListKeyDown(wxListEvent &event);
    void OnChar(wxListEvent &event);
    void OnColClick(wxListEvent &event);
    void OnSelected(wxListEvent &event);
    void OnDeselected(wxListEvent &event);
    void OnActivated(wxListEvent &event);
    void OnInsertItem(wxListEvent &event);
    void OnDeleteItem(wxListEvent &event);

    FileContainerIf *m_container;
    wxMenu      *m_popupMenu;
    int     m_totalSize;
    mutable BObserver *m_statusbarObserver;

    DECLARE_EVENT_TABLE()
};

#define IMPLEMENT_MODIFY_PROPERTY(methodName, property, boolValue)  \
    void FlexDiskListCtrl::methodName(wxCommandEvent& WXUNUSED(event))  \
    {                                   \
        SetPropertyOnSelectedItems(property, boolValue);        \
    }

#define IMPLEMENT_SIMPLE_MENUCOMMAND(eventMethod, implMethod)       \
    void FlexDiskListCtrl::eventMethod(wxCommandEvent& WXUNUSED(event)) \
    {                                   \
        implMethod();                           \
    }

#define IMPLEMENT_SORT_MENUCOMMAND(eventMethod, id)         \
    void FlexDiskListCtrl::eventMethod(wxCommandEvent& WXUNUSED(event)) \
    {                                   \
        SortItems(compareFlexListItems, id);                \
    }
#endif

