/*
    soptions.h


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



#ifndef SOPTIONS_INCLUDED
#define SOPTIONS_INCLUDED

#include "efiletim.h"
#include "e2.h"
#include <filesystem>
#include <string>
#include <array>
#include <vector>
#include <map>

namespace fs = std::filesystem;


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
    IsRamExt2x384,
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
    IconSize,
    FileTimeAccess,
    IsDisplaySmooth,
    PrintFont,
    IsPrintPageBreakDetected,
    PrintOrientation,
    PrintPageSize,
    PrintUnit,
    MainWindowGeometry,
    CpuDialogGeometry,
    PrintOutputWindowGeometry,
    PrintPreviewDialogGeometry,
    PrintConfigs,
    DirectoryDiskTrkSec,
    IsDirectoryDiskActive,
    IsStatusBarVisible,
    IsMagneticMainWindow,
    TerminalType,
    IsConfirmExit,
    MemoryWindowConfigs,
};
using FlexemuOptionIds_t = std::vector<FlexemuOptionId>;

extern const FlexemuOptionIds_t &GetAllFlexemuOptionIds();

struct sOptions
{
    sOptions() = default;
    sOptions(const sOptions &src) = default;
    // No glue why clang-tidy finds an identifier __i0 here.
    // NOLINTNEXTLINE(bugprone-reserved-identifier)
    sOptions& operator=(const sOptions &src) = default;

    std::string version;
    std::array<fs::path, MAX_DRIVES> drives;
    std::array<fs::path, 2> mdcrDrives;
    fs::path hex_file;
    fs::path disk_dir;
    std::string startup_command;
    bool isRamExtension{}; // Use RAM extension cards/No RAM extension
    bool isHiMem{}; // Use 2 x 384 KByte RAM extension/2 x 96 KByte RAM ext.
    bool isFlexibleMmu{}; // Use flexible MMU/Normal MMU
    bool isEurocom2V5{}; // Emulate an Eurocom II/V5 (instead of Eurocom II/V7)
    bool use_undocumented{};
    bool useRtc{};
    bool term_mode{};
    std::array<bool, MAX_DRIVES> canFormatDrives{};
    bool isTerminalIgnoreESC{}; // Terminal mode: Ignore ESC (0x1B) characters
    bool isTerminalIgnoreNUL{}; // Terminal mode: Ignore NUL (0x00) characters
    int terminalType{}; // type of terminal: 1 = scrolling, 2 = ncurses
    FileTimeAccess fileTimeAccess{};
    short int reset_key{};
    int iconSize{}; // Size of icons {16, 24, 32 }.
    float frequency{};

    // User interface options
    std::string color; // color name or "default" for multi color palette.
    fs::path doc_dir; // Directory containing html documenation.
    int nColors{}; // Number of colors or gray scale values { 2, 8, 64 }.
    bool isInverse{}; // Display inverse colors or gray scale values.
    bool isSmooth{}; // Display mode is smooth display.
    bool isConfirmExit{}; // On flexemu exit open confirmation dialog.
    int pixelSize{}; // Size of one pixel on the screen { 1, 2, 3, 4, 5 }.
                   // It depends on the screen dimensions on which flexemu
                   // is executed.
    std::string printFont; // Font used for printing documents (monospace)
    bool isPrintPageBreakDetected{}; // Print preview: Automatic page break det.
    std::string printOrientation; // Print preview: Page orientation (Port/Land)
    std::string printPageSize; // Print preview: Page size
    std::string printUnit; // Print preview: Unit (mm/inch)
    // Print preview options: Margins and size factor.
    // Orientation, page size and font are used as a key into the dictionary.
    std::map<std::string, std::string> printConfigs;
    std::string mainWindowGeometry; // Geometry of main window
    std::string cpuDialogGeometry; // Geometry of CPU dialog
    std::string printOutputWindowGeometry; // Geometry of print output window
    std::string printPreviewDialogGeometry; // Geometry of print preview dialog

    int directoryDiskTracks{}; // Default number of track for a directory disk
    int directoryDiskSectors{}; // Default number of sectors for a directory disk
    bool isDirectoryDiskActive{}; // true if directory disk is active.
    bool isStatusBarVisible{}; // true if status bar is visible.
    bool isMagneticMainWindow{}; // true if main window moves together with all
                                 // open windows.
    fs::path cpuLogPath; // Path used for CPU instruction logging

    FlexemuOptionIds_t readOnlyOptionIds;// List of option ids which are
                                         // read-only.
    std::vector<std::string> memoryWindowConfigs; // configuration of all
                                                  // memory windows.

    static const int maxMemoryWindows = 24;
};

#endif

