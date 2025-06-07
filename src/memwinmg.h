/*
    memwinmg.h


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

#ifndef MEMORYWINDOWMANAGER_INCLUDED
#define MEMORYWINDOWMANAGER_INCLUDED

#include "warnoff.h"
#include <QObject>
#include "warnon.h"
#include <memory>
#include <vector>


class Memory;
class Scheduler;
class QSize;
class CCopyMemory;
class MemoryWindow;
struct sOptions;

using CCopyMemorySPtr = std::shared_ptr<CCopyMemory>;
using MemoryWindowSPtr = std::shared_ptr<MemoryWindow>;

class MemoryWindowManager : public QObject
{
    Q_OBJECT

public:
    MemoryWindowManager() = default;
    ~MemoryWindowManager() override = default;

    void OpenMemoryWindow(bool isReadOnly, const sOptions &options,
            Memory &memory, Scheduler &scheduler);
    void RequestMemoryUpdate(Scheduler &scheduler) const;
    void UpdateData() const;
    void SetIconSize(const QSize &iconSize) const;
    void CloseAllWindows();
    void SetReadOnly(bool isReadOnly) const;

protected slots:
    void OnMemoryWindowClosed(const MemoryWindow *memoryWindow);

protected:
    struct MemoryWindowItem
    {
        MemoryWindowSPtr window;
        CCopyMemorySPtr copyMemoryCommand;
    };

    std::vector<MemoryWindowItem> items;
};

#endif

