/*
    soptions.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2021-2024  W. Schwotzer

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



#ifndef SOPTIONS_INCLUDED
#define SOPTIONS_INCLUDED

#include <string>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include "efiletim.h"

// Maximum size of one emulated pixel on screen
#define MAX_PIXELSIZE (5)

enum class FlexemuOptionId : uint8_t
{
    Drive0,
    Drive1,
    Drive2,
    Drive3,
    CanFormatDrive0,
    CanFormatDrive1,
    CanFormatDrive2,
    CanFormatDrive3,
    MdcrDrive0,
    MdcrDrive1,
    HexFile,
    DiskDirectory,
    IsRamExt2x96,
    IsRamExt2x288,
    IsFlexibleMmu,
    IsEurocom2V5,
    IsUseUndocumented,
    IsUseRtc,
    IsTerminalIgnoreESC,
    IsTerminalIgnoreNUL,
    Frequency,
    Color,
    NColors,
    IsInverse,
    PixelSize,
    FileTimeAccess,
    IsDisplaySmooth,
    PrintFont,
    IsPrintPageBreakDetected,
    PrintOrientation,
    PrintPageSize,
    PrintUnit,
    PrintOutputWindowGeometry,
    PrintPreviewDialogGeometry,
    PrintConfigs,
};
using FlexemuOptionIds = std::vector<FlexemuOptionId>;

const FlexemuOptionIds allFlexemuOptionIds {
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
    FlexemuOptionId::IsRamExt2x288,
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
    FlexemuOptionId::FileTimeAccess,
    FlexemuOptionId::IsDisplaySmooth,
    FlexemuOptionId::PrintFont,
    FlexemuOptionId::IsPrintPageBreakDetected,
    FlexemuOptionId::PrintOrientation,
    FlexemuOptionId::PrintPageSize,
    FlexemuOptionId::PrintUnit,
    FlexemuOptionId::PrintOutputWindowGeometry,
    FlexemuOptionId::PrintPreviewDialogGeometry,
    FlexemuOptionId::PrintConfigs,
};

struct sOptions
{
    sOptions() = default;
    sOptions(const sOptions &src) = default;
    sOptions& operator=(const sOptions &src) = default;

    std::string drive[4];
    std::array<std::string, 2> mdcrDrives;
    std::string hex_file;
    std::string disk_dir;
    std::string startup_command;
    bool isRamExtension;  // Use RAM extension cards/No RAM extension
    bool isHiMem;         // Use 2 x 288K RAM extension/2 x 96 K RAM ext.
    bool isFlexibleMmu;   // Use flexible MMU/Normal MMU
    bool isEurocom2V5;    // Emulate an Eurocom II/V5 (instead of Eurocom II/V7)
    bool use_undocumented;
    bool useRtc;
    bool term_mode;
    bool canFormatDrive[4];
    bool isTerminalIgnoreESC; // Terminal mode: Ignore ESC (0x1B) characters
    bool isTerminalIgnoreNUL; // Terminal mode: Ignore NUL (0x00) characters
    FileTimeAccess fileTimeAccess;
    short int reset_key; // must be short int because of sscanf !!!
    float frequency;

    // User interface options
    std::string color; // color name or "default" for multi color palette.
    std::string doc_dir; // Directory containing html documenation.
    int nColors; // Number of colors or gray scale values { 2, 8, 64 }.
    bool isInverse; // Display inverse colors or gray scale values.
    bool isSmooth; // Display mode is smooth display.
    int pixelSize; // Size of one pixel on the screen { 1, 2, 3, 4, 5 }.
                   // It depends on the screen dimensions on which flexemu
                   // is executed.
    std::string printFont; // Font used for printing documents (monospace)
    bool isPrintPageBreakDetected; // Print preview: Automatic page break det.
    std::string printOrientation; // Print preview: Page orientation (Port/Land)
    std::string printPageSize; // Print preview: Page size
    std::string printUnit; // Print preview: Unit (mm/inch)
    // Print preview options: Margins and size factor.
    // Orientation, page size and font are used as a key into the dictionary.
    std::map<std::string, std::string> printConfigs;
    std::string printOutputWindowGeometry; // Geometry of print output window
    std::string printPreviewDialogGeometry; // Geometry of print preview dialog

    FlexemuOptionIds readOnlyOptionIds;// List of option ids which are
                                       // read-only.
};

#endif

