/*
    memwinmg.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#include "memwinmg.h"
#include "memwin.h"
#include "memsetui.h"
#include "bintervl.h"
#include "ccopymem.h"
#include "memory.h"
#include "schedule.h"
#include "soptions.h"
#include "warnoff.h"
#include <QSize>
#include <QString>
#include <QDialog>
#include "warnon.h"
#include <memory>
#include <cassert>

/********************
** Protected slots **
********************/
void MemoryWindowManager::OnMemoryWindowClosed(
        const MemoryWindow *memoryWindow)
{
      for (auto iter = items.cbegin(); iter != items.cend(); ++iter)
     {
         if (iter->window.get() == memoryWindow)
         {
             items.erase(iter);
             break;
         }
     }
}

/****************************
** Public member functions **
****************************/
void MemoryWindowManager::OpenMemoryWindow(
        bool isReadOnly,
        const sOptions &options,
        Memory &memory,
        Scheduler &scheduler)
{
     BInterval<DWord> addressRange = { 0xC100U, 0xC6FFU };
     QString windowTitle;
     auto style = MemoryWindow::Style::Bytes16;
     bool withAddress = true;
     bool withAscii = true;
     bool withExtraSpace = false;
     bool isUpdateWindowSize = true;
     auto *dialog = new QDialog;
     MemorySettingsUi ui;

     ui.setupUi(*dialog);
     ui.SetData(addressRange, windowTitle, style, withAddress, withAscii,
                withExtraSpace, isUpdateWindowSize);
     dialog->adjustSize();
     auto result = dialog->exec();
     if (result != QDialog::Accepted)
     {
         return;
     }

     ui.GetData(addressRange, windowTitle, style, withAddress, withAscii,
                withExtraSpace, isUpdateWindowSize);
     auto readMemoryCommand =
         std::make_shared<CReadMemory>(memory, addressRange);
     auto window = std::make_unique<MemoryWindow>(
             isReadOnly, addressRange, windowTitle, style, withAddress,
             withAscii, withExtraSpace, isUpdateWindowSize);
     window->SetIconSize({ options.iconSize, options.iconSize });
     window->show();
     connect(window.get(), &MemoryWindow::Closed,
             this, &MemoryWindowManager::OnMemoryWindowClosed);
     MemoryWindowItem item = { std::move(window), readMemoryCommand };
     items.push_back(std::move(item));

     scheduler.sync_exec(
         std::dynamic_pointer_cast<BCommand>(readMemoryCommand));
}

void MemoryWindowManager::RequestMemoryUpdate(Scheduler &scheduler) const
{
    for (const auto &item : items)
    {
         scheduler.sync_exec(std::dynamic_pointer_cast<BCommand>(
                     item.readMemoryCommand));
     }
}

void MemoryWindowManager::UpdateData() const
{
    for (const auto &item : items)
    {
        if (item.readMemoryCommand->HasUpdate())
        {
            item.window->UpdateData(item.readMemoryCommand->GetData());
        }
    }
}

void MemoryWindowManager::SetIconSize(const QSize &iconSize) const
{
    for (const auto &item : items)
    {
        item.window->SetIconSize(iconSize);
    }
}

void MemoryWindowManager::CloseAllWindows()
{
    // Iterate on a copy because for each close() OnMemoryWindowClosed()
    // slot is called which modifies items.
    std::vector<MemoryWindowItem> itemsCopy = items;

    for (auto &item : itemsCopy)
    {
        item.window->close();
    }
}

void MemoryWindowManager::SetReadOnly(bool isReadOnly) const
{
    for (const auto &item : items)
    {
        item.window->SetReadOnly(isReadOnly);
    }
}
