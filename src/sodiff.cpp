/*
    sodiff.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2021-2025  W. Schwotzer

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


#include "sodiff.h"
#include <algorithm>

FlexemuOptionsDifference::FlexemuOptionsDifference(
        const sOptions &opt1, const sOptions &opt2)
{
    if (opt1.drives[0] != opt2.drives[0])
    {
        notEquals.push_back(FlexemuOptionId::Drive0);
    }

    if (opt1.drives[1] != opt2.drives[1])
    {
        notEquals.push_back(FlexemuOptionId::Drive1);
    }

    if (opt1.drives[2] != opt2.drives[2])
    {
        notEquals.push_back(FlexemuOptionId::Drive2);
    }

    if (opt1.drives[3] != opt2.drives[3])
    {
        notEquals.push_back(FlexemuOptionId::Drive3);
    }

    if (opt1.canFormatDrives[0] != opt2.canFormatDrives[0])
    {
        notEquals.push_back(FlexemuOptionId::CanFormatDrive0);
    }

    if (opt1.canFormatDrives[1] != opt2.canFormatDrives[1])
    {
        notEquals.push_back(FlexemuOptionId::CanFormatDrive1);
    }

    if (opt1.canFormatDrives[2] != opt2.canFormatDrives[2])
    {
        notEquals.push_back(FlexemuOptionId::CanFormatDrive2);
    }

    if (opt1.canFormatDrives[3] != opt2.canFormatDrives[3])
    {
        notEquals.push_back(FlexemuOptionId::CanFormatDrive3);
    }

    if (opt1.mdcrDrives[0] != opt2.mdcrDrives[0])
    {
        notEquals.push_back(FlexemuOptionId::MdcrDrive0);
    }

    if (opt1.mdcrDrives[1] != opt2.mdcrDrives[1])
    {
        notEquals.push_back(FlexemuOptionId::MdcrDrive1);
    }

    if (opt1.hex_file != opt2.hex_file)
    {
        notEquals.push_back(FlexemuOptionId::HexFile);
    }

    if (opt1.disk_dir != opt2.disk_dir)
    {
        notEquals.push_back(FlexemuOptionId::DiskDirectory);
    }

    if (opt1.isRamExtension != opt2.isRamExtension)
    {
        notEquals.push_back(FlexemuOptionId::IsRamExt2x96);
    }

    if (opt1.isHiMem != opt2.isHiMem)
    {
        notEquals.push_back(FlexemuOptionId::IsRamExt2x288);
    }

    if (opt1.isFlexibleMmu != opt2.isFlexibleMmu)
    {
        notEquals.push_back(FlexemuOptionId::IsFlexibleMmu);
    }

    if (opt1.isEurocom2V5 != opt2.isEurocom2V5)
    {
        notEquals.push_back(FlexemuOptionId::IsEurocom2V5);
    }

    if (opt1.use_undocumented != opt2.use_undocumented)
    {
        notEquals.push_back(FlexemuOptionId::IsUseUndocumented);
    }

    if (opt1.useRtc != opt2.useRtc)
    {
        notEquals.push_back(FlexemuOptionId::IsUseRtc);
    }

    if (opt1.frequency != opt2.frequency)
    {
        notEquals.push_back(FlexemuOptionId::Frequency);
    }

    if (opt1.color != opt2.color)
    {
        notEquals.push_back(FlexemuOptionId::Color);
    }

    if (opt1.nColors != opt2.nColors)
    {
        notEquals.push_back(FlexemuOptionId::NColors);
    }

    if (opt1.isInverse != opt2.isInverse)
    {
        notEquals.push_back(FlexemuOptionId::IsInverse);
    }

    if (opt1.pixelSize != opt2.pixelSize)
    {
        notEquals.push_back(FlexemuOptionId::PixelSize);
    }

    if (opt1.iconSize != opt2.iconSize)
    {
        notEquals.push_back(FlexemuOptionId::IconSize);
    }

    if (opt1.fileTimeAccess != opt2.fileTimeAccess)
    {
        notEquals.push_back(FlexemuOptionId::FileTimeAccess);
    }

    if (opt1.isSmooth != opt2.isSmooth)
    {
        notEquals.push_back(FlexemuOptionId::IsDisplaySmooth);
    }

    if (opt1.isConfirmExit != opt2.isConfirmExit)
    {
        notEquals.push_back(FlexemuOptionId::IsConfirmExit);
    }

    if (opt1.isTerminalIgnoreNUL != opt2.isTerminalIgnoreNUL)
    {
        notEquals.push_back(FlexemuOptionId::IsTerminalIgnoreNUL);
    }

    if (opt1.isTerminalIgnoreESC != opt2.isTerminalIgnoreESC)
    {
        notEquals.push_back(FlexemuOptionId::IsTerminalIgnoreESC);
    }

    if (opt1.terminalType != opt2.terminalType)
    {
        notEquals.push_back(FlexemuOptionId::TerminalType);
    }

    if (opt1.isStatusBarVisible != opt2.isStatusBarVisible)
    {
        notEquals.push_back(FlexemuOptionId::IsStatusBarVisible);
    }

    if (opt1.isDirectoryDiskActive != opt2.isDirectoryDiskActive)
    {
        notEquals.push_back(FlexemuOptionId::IsDirectoryDiskActive);
    }

    if (opt1.directoryDiskTracks != opt2.directoryDiskTracks ||
        opt1.directoryDiskSectors != opt2.directoryDiskSectors)
    {
        notEquals.push_back(FlexemuOptionId::DirectoryDiskTrkSec);
    }

    if (opt1.printFont != opt2.printFont)
    {
        notEquals.push_back(FlexemuOptionId::PrintFont);
    }

    if (opt1.isPrintPageBreakDetected != opt2.isPrintPageBreakDetected)
    {
        notEquals.push_back(FlexemuOptionId::IsPrintPageBreakDetected);
    }

    if (opt1.printOrientation != opt2.printOrientation)
    {
        notEquals.push_back(FlexemuOptionId::PrintOrientation);
    }

    if (opt1.printPageSize != opt2.printPageSize)
    {
        notEquals.push_back(FlexemuOptionId::PrintPageSize);
    }

    if (opt1.printUnit != opt2.printUnit)
    {
        notEquals.push_back(FlexemuOptionId::PrintUnit);
    }

    if (opt1.printOutputWindowGeometry != opt2.printOutputWindowGeometry)
    {
        notEquals.push_back(FlexemuOptionId::PrintOutputWindowGeometry);
    }

    if (opt1.printPreviewDialogGeometry != opt2.printPreviewDialogGeometry)
    {
        notEquals.push_back(FlexemuOptionId::PrintPreviewDialogGeometry);
    }

    if (opt1.printConfigs != opt2.printConfigs)
    {
        notEquals.push_back(FlexemuOptionId::PrintConfigs);
    }
}

const FlexemuOptionsDifference::Result&
    FlexemuOptionsDifference::GetNotEquals() const
{
    return notEquals;
}

bool IsRestartNeeded(const FlexemuOptionsDifference &optionsDiff)
{
    static const FlexemuOptionIds_t restartOptionIds{
        FlexemuOptionId::Drive0,
        FlexemuOptionId::Drive1,
        FlexemuOptionId::Drive2,
        FlexemuOptionId::Drive3,
        FlexemuOptionId::MdcrDrive0,
        FlexemuOptionId::MdcrDrive1,
        FlexemuOptionId::HexFile,
        FlexemuOptionId::DiskDirectory,
        FlexemuOptionId::IsRamExt2x96,
        FlexemuOptionId::IsRamExt2x288,
        FlexemuOptionId::IsFlexibleMmu,
        FlexemuOptionId::IsEurocom2V5,
        FlexemuOptionId::IsUseRtc,
        FlexemuOptionId::IsDirectoryDiskActive,
        FlexemuOptionId::TerminalType,
    };

    for (auto id : optionsDiff.GetNotEquals())
    {
        if (std::any_of(restartOptionIds.cbegin(), restartOptionIds.cend(),
            [&](FlexemuOptionId restartOptionId)
            {
                return  id == restartOptionId;
            }))
        {
            return true;
        }
    }

    return false;
}

