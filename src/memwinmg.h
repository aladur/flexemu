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

#include "typedefs.h"
#include "warnoff.h"
#include <QObject>
#include "warnon.h"
#include "bintervl.h"
#include "memwin.h"
#include <memory>
#include <vector>


class Memory;
class Scheduler;
class QSize;
class QWidget;
class CReadMemory;
class CWriteMemory;
class MemoryWindowManager;
struct sOptions;

using CReadMemorySPtr = std::shared_ptr<CReadMemory>;
using MemoryWindowManagerSPtr = std::shared_ptr<MemoryWindowManager>;
using MemoryWindowSPtr = std::shared_ptr<MemoryWindow>;
using MemoryWindowSPtrs = std::vector<MemoryWindowSPtr>;

class MemoryWindowManager : public QObject
{
    Q_OBJECT

public:
    explicit MemoryWindowManager(QWidget *p_parent, Scheduler &p_scheduler,
            Memory &p_memory);
    MemoryWindowManager() = delete;
    ~MemoryWindowManager() override = default;
    MemoryWindowManager(const MemoryWindowManager &src) = delete;
    MemoryWindowManager(MemoryWindowManager &&src) = delete;
    MemoryWindowManager &operator=(const MemoryWindowManager &src) = delete;
    MemoryWindowManager &operator=(MemoryWindowManager &&src) = delete;

    void OpenMemoryWindow(bool isReadOnly, const sOptions &options);
    void RequestMemoryUpdate(
            const BInterval<DWord> &addressRange =
                BInterval<DWord>(0x0000U, 0xFFFFU),
            const MemoryWindow *excludeWindow = nullptr) const;
    void UpdateData() const;
    void SetIconSize(const QSize &iconSize) const;
    void CloseAllWindows(sOptions &options);
    void OpenAllWindows(bool isReadOnly, const sOptions &options);
    MemoryWindowSPtrs GetAllWindows() const;
    void SetReadOnly(bool isReadOnly) const;

protected slots:
    void OnMemoryWindowClosed(const MemoryWindow *memoryWindow);
    void OnMemoryModified(const MemoryWindow *memoryWindow, DWord address,
            const std::vector<Byte> &data);

protected:
    void OpenMemoryWindow(bool isReadOnly,
            const sOptions &options,
            const MemoryWindow::Config_t &config,
            std::optional<QRect> positionAndSize = std::nullopt);

private:
    struct MemoryWindowItem
    {
        MemoryWindowSPtr window;
        CReadMemorySPtr readMemoryCommand;
    };

    std::vector<MemoryWindowItem> items;
    Scheduler &scheduler;
    Memory &memory;
    QWidget *parent{};
};

#endif

