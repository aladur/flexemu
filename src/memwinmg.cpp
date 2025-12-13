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


#include "config.h"
#include "typedefs.h"
#include "misc1.h"
#include "memwinmg.h"
#include "bcommand.h"
#include "free.h"
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
#include <QPoint>
#include <QString>
#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QMessageBox>
#include <fmt/format.h>
#include "warnon.h"
#include <cassert>
#include <memory>
#include <optional>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

MemoryWindowManager::MemoryWindowManager(QWidget *p_parent,
        Scheduler &p_scheduler,
        Memory &p_memory)
    : scheduler(p_scheduler)
    , memory(p_memory)
    , parent(p_parent)
{
}

/********************
** Protected slots **
********************/
void MemoryWindowManager::OnMemoryWindowActivated(
        const MemoryWindow *memoryWindow)
{
    for (auto iter = windowsInZOrder.begin(); iter != windowsInZOrder.end();
         ++iter)
    {
        if (iter->get() == memoryWindow &&
            iter->get() != windowsInZOrder.back().get())
        {
            auto activatedWindow = *iter;
            windowsInZOrder.erase(iter);
            windowsInZOrder.emplace_back(std::move(activatedWindow));
            break;
        }
    }

    assert(items.size() == windowsInZOrder.size());
}

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

    for (auto iter = windowsInZOrder.cbegin(); iter != windowsInZOrder.cend();
         ++iter)
    {
        if (iter->get() == memoryWindow)
        {
            windowsInZOrder.erase(iter);
            break;
        }
    }

    assert(items.size() == windowsInZOrder.size());
}

void MemoryWindowManager::OnMemoryModified(const MemoryWindow *memoryWindow,
        DWord address, const std::vector<Byte> &data)
{
    if (!data.empty())
    {
        const auto writeMemoryCommand =
            std::make_shared<CWriteMemory>(memory, static_cast<Word>(address),
                    data);
        scheduler.sync_exec(
            std::dynamic_pointer_cast<BCommand>(writeMemoryCommand));

        const BInterval<DWord> addressRange(address,
            static_cast<DWord>(address + data.size() - 1U));
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
     auto *dialog = new QDialog(parent);
     MemorySettingsUi ui;

     if (items.size() >= sOptions::maxMemoryWindows)
     {
        const auto msg = QString(
                "A maximum of %1 Memory Windows are open.\n"
                "No more Memory Windows can be opened.")
                 .arg(sOptions::maxMemoryWindows);
         QMessageBox::warning(parent, PACKAGE_NAME " warning", msg);
         return;
     }

     ui.setupUi(*dialog);
     ui.SetData(config);
     dialog->adjustSize();
     auto result = dialog->exec();
     if (result != QDialog::Accepted)
     {
         return;
     }

     ui.GetData(config);

     OpenMemoryWindow(isReadOnly, options, config);
}

bool MemoryWindowManager::HasValidAddressRange(
        const MemoryWindow::Config_t &config) const
{
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

        const auto range = QString("%1-%2")
            .arg(config.addressRange.lower(), 4, 16, QLatin1Char( '0' ))
            .arg(config.addressRange.upper(), 4, 16, QLatin1Char( '0' ))
            .toUpper();
        QString message =
            tr("Memory Window has invalid address range:\n") + range +
            tr(".\n\nValid address ranges:") +
            QString::fromStdString(stream.str());
        QMessageBox::critical(parent, tr("flexemu error"), message);

        return false;
    }

    return true;
}

void MemoryWindowManager::OpenMemoryWindow(bool isReadOnly,
        const sOptions &options,
        const MemoryWindow::Config_t &config,
        const std::optional<QRect> positionAndSize)
{
     if (!HasValidAddressRange(config))
     {
         return;
     }

     auto readMemoryCommand =
         std::make_shared<CReadMemory>(memory, config.addressRange);
     auto window = std::make_shared<MemoryWindow>( isReadOnly,
             memory.GetMemoryRanges(), config, positionAndSize, parent);
     window->SetIconSize({ options.iconSize, options.iconSize });
     windowsInZOrder.push_back(window);
     window->show();
     connect(window.get(), &MemoryWindow::Activated,
             this, &MemoryWindowManager::OnMemoryWindowActivated);
     connect(window.get(), &MemoryWindow::Closed,
             this, &MemoryWindowManager::OnMemoryWindowClosed);
     connect(window.get(), &MemoryWindow::MemoryModified,
             this, &MemoryWindowManager::OnMemoryModified);
     MemoryWindowItem item = { window, readMemoryCommand };
     items.push_back(std::move(item));

     assert(items.size() == windowsInZOrder.size());

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

void MemoryWindowManager::CloseAllWindows(sOptions &options)
{
    // Iterate on a copy because for each close() OnMemoryWindowClosed()
    // slot is called which modifies items and windowsInZOrder.
    const auto itemsCopy = items;
    const auto windowsInZOrderCopy = windowsInZOrder;
    std::map<MemoryWindow *, unsigned> indexForWindow;
    unsigned index = 0U;

    assert(items.size() == windowsInZOrder.size());

    for(const auto &item : itemsCopy)
    {
        indexForWindow[item.window.get()] = index++;
    }

    options.memoryWindowConfigs.clear();
    // Collect config from lowest to highest z-order.
    for (const auto &window : windowsInZOrderCopy)
    {
        index = indexForWindow[window.get()];
        auto configString = itemsCopy[index].window->GetConfigString();
        options.memoryWindowConfigs.emplace_back(configString);
        itemsCopy[index].window->close();
    }

    windowsInZOrder.clear();
    items.clear();
}

void MemoryWindowManager::OpenAllWindows(bool isReadOnly,
        const sOptions &options)
{
    for (const auto &configString : options.memoryWindowConfigs)
    {
        MemoryWindow::Config_t config;
        QRect positionAndSize{};

        const auto count = MemoryWindow::ConvertConfigString(configString,
                config, positionAndSize);
        if (items.size() >= sOptions::maxMemoryWindows)
        {
            return;
        }

        // Check for valid addr. range and minimum config parameters.
        if (!HasValidAddressRange(config) || count < 7U)
        {
            continue;
        }

        QTimer::singleShot(0, this, [&, config, positionAndSize, isReadOnly](){

            OpenMemoryWindow(isReadOnly, options, config, positionAndSize);
        });
    }
}

void MemoryWindowManager::MoveAllWindows(const QPoint &diffPos)
{
    for (const auto &item : items)
    {
        item.window->move(item.window->pos() + diffPos);
    }
}

MemoryWindowSPtrs MemoryWindowManager::GetAllWindows() const
{
    MemoryWindowSPtrs result;

    std::transform(items.cbegin(), items.cend(), std::back_inserter(result),
        [&](const MemoryWindowItem &item)
        {
            return item.window;
        });

    struct
    {
        bool operator()(MemoryWindowSPtr &lhs, MemoryWindowSPtr &rhs)
        {
            if (lhs->GetAddressRange().lower() ==
                rhs->GetAddressRange().lower())
            {
                return lhs->GetAddressRange().upper() <
                       rhs->GetAddressRange().upper();
            }

            return lhs->GetAddressRange().lower() <
                   rhs->GetAddressRange().lower();
        };
    } compareMemoryWindows;

    std::sort(result.begin(), result.end(), compareMemoryWindows);

    return result;
}

void MemoryWindowManager::SetReadOnly(bool isReadOnly) const
{
    for (const auto &item : items)
    {
        item.window->SetReadOnly(isReadOnly);
    }
}
