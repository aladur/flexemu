/*
    soptions.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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


#include "soptions.h"


const FlexemuOptionIds_t &GetAllFlexemuOptionIds()
{
    static const FlexemuOptionIds_t allFlexemuOptionIds{
        FlexemuOptionId::Drive0,
        FlexemuOptionId::Drive1,
        FlexemuOptionId::Drive2,
        FlexemuOptionId::Drive3,
        FlexemuOptionId::CanFormatDrive0,
        FlexemuOptionId::CanFormatDrive1,
        FlexemuOptionId::CanFormatDrive2,
        FlexemuOptionId::CanFormatDrive3,
        FlexemuOptionId::MdcrDrive0,
        FlexemuOptionId::MdcrDrive1,
        FlexemuOptionId::HexFile,
        FlexemuOptionId::DiskDirectory,
        FlexemuOptionId::IsRamExt2x96,
        FlexemuOptionId::IsRamExt2x384,
        FlexemuOptionId::IsFlexibleMmu,
        FlexemuOptionId::IsEurocom2V5,
        FlexemuOptionId::IsUseUndocumented,
        FlexemuOptionId::IsUseRtc,
        FlexemuOptionId::IsTerminalIgnoreESC,
        FlexemuOptionId::IsTerminalIgnoreNUL,
        FlexemuOptionId::Frequency,
        FlexemuOptionId::Color,
        FlexemuOptionId::NColors,
        FlexemuOptionId::IsInverse,
        FlexemuOptionId::PixelSize,
        FlexemuOptionId::IconSize,
        FlexemuOptionId::FileTimeAccess,
        FlexemuOptionId::IsDisplaySmooth,
        FlexemuOptionId::IsConfirmExit,
        FlexemuOptionId::PrintFont,
        FlexemuOptionId::IsPrintPageBreakDetected,
        FlexemuOptionId::PrintOrientation,
        FlexemuOptionId::PrintPageSize,
        FlexemuOptionId::PrintUnit,
        FlexemuOptionId::MainWindowGeometry,
        FlexemuOptionId::CpuDialogGeometry,
        FlexemuOptionId::PrintOutputWindowGeometry,
        FlexemuOptionId::PrintPreviewDialogGeometry,
        FlexemuOptionId::PrintConfigs,
        FlexemuOptionId::DirectoryDiskTrkSec,
        FlexemuOptionId::IsStatusBarVisible,
        FlexemuOptionId::IsMagneticMainWindow,
        FlexemuOptionId::IsFullscreen,
        FlexemuOptionId::TerminalType,
        FlexemuOptionId::MemoryWindowConfigs,
    };

    return allFlexemuOptionIds;
};

