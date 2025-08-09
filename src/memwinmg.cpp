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
#include "cwritmem.h"
#include "memory.h"
#include "schedule.h"
#include "soptions.h"
#include "warnoff.h"
#include <QSize>
#include <QString>
#include <QDialog>
#include <QMessageBox>
#include <fmt/format.h>
#include "warnon.h"
#include <memory>
#include <cassert>
#include <sstream>

MemoryWindowManager::MemoryWindowManager(Scheduler &p_scheduler,
        Memory &p_memory)
    : scheduler(p_scheduler)
    , memory(p_memory)
{
}

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

void MemoryWindowManager::OnMemoryModified(const MemoryWindow *memoryWindow,
        Word address, const std::vector<Byte> &data)
{
    if (!data.empty())
    {
        const auto writeMemoryCommand =
            std::make_shared<CWriteMemory>(memory, address, data);
        scheduler.sync_exec(
            std::dynamic_pointer_cast<BCommand>(writeMemoryCommand));

        const BInterval<DWord> addressRange(address,
            address + data.size() - 1U);
        RequestMemoryUpdate(addressRange, memoryWindow);
    }
}

/****************************
** Public member functions **
****************************/
void MemoryWindowManager::OpenMemoryWindow(bool isReadOnly,
        const sOptions &options)
{
     MemoryWindow::Config_t config{"", { 0xC100U, 0xC6FFU },
        MemoryWindow::Style::Bytes16, true, true, false, true};
     auto *dialog = new QDialog;
     MemorySettingsUi ui;

     ui.setupUi(*dialog);
     ui.SetData(config);
     dialog->adjustSize();
     auto result = dialog->exec();
     if (result != QDialog::Accepted)
     {
         return;
     }

     ui.GetData(config);

     if (!flx::is_range_in_ranges(config.addressRange,
                 memory.GetAddressRanges()))
     {
         std::stringstream stream;

         for (const auto &item : memory.GetMemoryRanges())
         {
            if (item.type != MemoryType::NONE)
            {
                std::stringstream typeStream;

                typeStream << item.type;
                stream <<
                    fmt::format("\n{}: {:04X}-{:04X}", typeStream.str(),
                        item.addressRange.lower(), item.addressRange.upper());
            }
         }

         QString message =
             tr("Invalid address range specified.\n\nValid address ranges:") +
             QString::fromStdString(stream.str());
         QMessageBox::critical(nullptr, tr("flexemu error"), message);
         return;
     }

     auto readMemoryCommand =
         std::make_shared<CReadMemory>(memory, config.addressRange);
     auto window = std::make_unique<MemoryWindow>(
             isReadOnly, memory.GetMemoryRanges(), config);
     window->SetIconSize({ options.iconSize, options.iconSize });
     window->show();
     connect(window.get(), &MemoryWindow::Closed,
             this, &MemoryWindowManager::OnMemoryWindowClosed);
     connect(window.get(), &MemoryWindow::MemoryModified,
             this, &MemoryWindowManager::OnMemoryModified);
     MemoryWindowItem item = { std::move(window), readMemoryCommand };
     items.push_back(std::move(item));

     scheduler.sync_exec(
         std::dynamic_pointer_cast<BCommand>(readMemoryCommand));
}

void MemoryWindowManager::RequestMemoryUpdate(
        const BInterval<DWord> &addressRange,
        const MemoryWindow *excludeWindow) const
{
    for (const auto &item : items)
    {
        if (item.window.get() != excludeWindow &&
            overlap(item.window->GetAddressRange(), addressRange))
        {
            scheduler.sync_exec(std::dynamic_pointer_cast<BCommand>(
                item.readMemoryCommand));
        }
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
