/*
    foptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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

#include "misc1.h"
#include <stdexcept>
#include <string>
#include <new>

#include "e2.h"
#include "flexemu.h"
#include "soptions.h"
#include "foptman.h"
#include "bregistr.h"
#include "brcfile.h"
#include <cstring>
#include <filesystem>
#include <sys/stat.h>


namespace fs = std::filesystem;

static const auto * const OLDFLEXEMURC = u8".flexemurc";
static const auto * const FLEXEMURC = u8"flexemurc";

static const char * const FLEXDISKDIR = "DiskDirectory";
static const char * const FLEXDISK0 = "Disk0Path";
static const char * const FLEXDISK1 = "Disk1Path";
static const char * const FLEXDISK2 = "Disk2Path";
static const char * const FLEXDISK3 = "Disk3Path";
static const char * const FLEXMDCRDRIVE0 = "MdcrDrive0Path";
static const char * const FLEXMDCRDRIVE1 = "MdcrDrive1Path";
static const char * const FLEXCOLOR = "DisplayColor";
static const char * const FLEXNCOLORS = "NoOfColors";
static const char * const FLEXINVERSE = "DisplayInverse";
static const char * const FLEXHIMEM = "HighMemory";
static const char * const FLEXFLEXIBLEMMU = "UseFlexibleMmu";
static const char * const FLEXRAMEXTENSION = "UseRamExtension";
static const char * const FLEXEUROCOM2V5 = "UseEurocom2V5";
static const char * const FLEXUNDOCUMENTED = "UndocumentedMc6809";
static const char * const FLEXRTC = "UseRTC";
static const char * const FLEXFORMATDRIVE0 = "CanFormatDrive0";
static const char * const FLEXFORMATDRIVE1 = "CanFormatDrive1";
static const char * const FLEXFORMATDRIVE2 = "CanFormatDrive2";
static const char * const FLEXFORMATDRIVE3 = "CanFormatDrive3";
#ifdef _WIN32
static const char * const FLEXDOCDIR = "DocDirectory";
#endif
static const char * const FLEXMONITOR = "MonitorPath";
static const char * const FLEXVERSION = "Version";
static const char * const FLEXSCREENFACTOR = "ScreenFactor";
static const char * const FLEXICONSIZE = "IconSize";
static const char * const FLEXISSTATUSBARVISIBLE = "IsStatusBarVisible";
static const char * const FLEXFREQUENCY = "Frequency";
static const char * const FLEXFILETIMEACCESS = "FileTimeAccess";
static const char * const FLEXDISPLAYSMOOTH = "DisplaySmooth";
static const char * const FLEXISCONFIRMEXIT = "IsConfirmExit";
static const char * const FLEXTERMINALIGNOREESC = "TerminalIgnoreESC";
static const char * const FLEXTERMINALIGNORENUL = "TerminalIgnoreNUL";
static const char * const FLEXTERMINALTYPE = "TerminalType";
static const char * const FLEXPRINTFONT = "PrintFont";
static const char * const FLEXPRINTPAGEBREAKDETECTED = "PrintPageBreakDetected";
static const char * const FLEXPRINTORIENTATION = "PrintOrientation";
static const char * const FLEXPRINTPAGESIZE = "PrintPageSize";
static const char * const FLEXPRINTUNIT = "PrintUnit";
static const char * const FLEXPRINTOUTPUTWINDOWGEOMETRY = "PrintOutputWindowGeometry";
static const char * const FLEXPRINTPREVIEWDIALOGGEOMETRY =
                    "PrintPreviewDialogGeometry";
static const char * const FLEXPRINTCONFIG = "PrintConfig";
static const char * const FLEXDIRECTORYDISKTRACKS = "DirectoryDiskTracks";
static const char * const FLEXDIRECTORYDISKSECTORS = "DirectoryDiskSectors";
static const char * const FLEXISDIRECTORYDISKACTIVE = "IsDirectoryDiskActive";

void FlexemuOptions::PrintHelp(std::ostream &os)
{
    os << "usage: flexemu <options>\n"
          "  <options> are:\n"
          "  -f <hexfile>\n"
          "  -0 <diskimage/directory drive 0>\n"
          "  -1 <diskimage/directory drive 1>\n"
          "  -2 <diskimage/directory drive 2>\n"
          "  -3 <diskimage/directory drive 3>\n"
          "  -p (directory for FLEX disks)\n"
          "  -j <factor for screen size>\n"
          "  -m (use 2 x 288 KByte RAM extension)\n"
          "  -u (support undocumented MC6809 processor instructions)\n"
          "  -F <frequency> (set CPU frequency in MHz)\n"
          "     0.0 sets maximum frequency, -1.0 sets original frequency.\n"
          "  -C <startup command>\n"
#ifdef UNIX
          "  -t (terminal only mode)\n"
#ifdef HAVE_TERMIOS_H
          "  -r <two-hex-digit reset key>\n"
#endif
          "  -T <terminal_type> ('scroll' or 'curses')\n"
#endif
          "  -c <color> define foreground color\n"
          "  -i (display inverse video)\n"
          "  -O <cccc> (Support formatting disk in drive 0..3\n"
          "     'c' represents drive 0..3 and can be: '0'=no; '1'=yes or "
            "'-'=unchanged,\n"
          "     Example: -O 001- Drive 2 allows format, drive 0,1 not, "
          "drive 3 unchanged.\n"
          "  -n <# of colors>\n"
          "  -L <file_path> Enable CPU instruction logging.\n"
          "     File extension: *.log or *.txt logs to a text file; "
          "*.csv logs to a csv file.\n"
          "  -h (display this)\n"
          "  -? (display this)\n"
          "  -V (print version number)\n";
}

void FlexemuOptions::InitOptions(struct sOptions &options)
{
    options.drives[0] = fs::u8path(u8"system.dsk");
    options.drives[1] = fs::u8path(u8"");
    options.drives[2] = fs::u8path(u8"");
    options.drives[3] = fs::u8path(u8"");
    options.mdcrDrives[0] = fs::u8path("system.mdcr");
    options.mdcrDrives[1] = fs::u8path(u8"");
    options.hex_file = fs::u8path(u8"neumon54.hex");
    options.startup_command = "";
    options.term_mode = false;
    options.isRamExtension = true;
    options.isHiMem = false;
    options.isFlexibleMmu = false;
    options.isEurocom2V5 = false;
    options.use_undocumented = false;
    options.useRtc = true;
    options.canFormatDrives[0] = false;
    options.canFormatDrives[1] = false;
    options.canFormatDrives[2] = false;
    options.canFormatDrives[3] = false;
    options.fileTimeAccess = FileTimeAccess::NONE;
    options.reset_key = 0x1e; // is Ctrl-^ for reset or Sig. INT
    options.frequency = -1.0; // default: ignore

    options.color = "green";
    options.nColors = 2;
    options.isInverse = false;
    options.isSmooth = false;
    options.isConfirmExit = true;
    options.isTerminalIgnoreESC = true;
    options.isTerminalIgnoreNUL = true;
#ifdef _WIN32
    options.terminalType = 0;
    options.doc_dir = flx::getExecutablePath() / u8"Documentation";
    options.disk_dir = flx::getExecutablePath() / u8"Data";
#else
    options.terminalType = 1;
    options.doc_dir = F_DATADIR;
    options.disk_dir = F_DATADIR;
#endif
    options.pixelSize = 2;
    options.iconSize = 16;
    options.readOnlyOptionIds.clear();
    options.isPrintPageBreakDetected = false;
    options.directoryDiskTracks = 80;
    options.directoryDiskSectors = 36;
    options.isDirectoryDiskActive = true;
    options.isStatusBarVisible = true;
}

void FlexemuOptions::GetCommandlineOptions(
    struct sOptions &options,
    int argc,
// Parameter comes from main().
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char *const argv[])
{
    int i;
    float f;
    optind = 1;
    opterr = 1;
    std::string optstr("mup:f:0:1:2:3:j:F:C:O:L:");
#ifdef HAVE_TERMIOS_H
    optstr.append("tr:T:"); // terminal mode, reset key and terminal type
#endif
    optstr.append("ic:n:"); // color, inverse video, # of colors
    optstr.append("Vh"); // version and help

    auto setReadOnly =
        [&roOptions = options.readOnlyOptionIds](FlexemuOptionId enumValue)
    {
        if (std::find(roOptions.cbegin(), roOptions.cend(), enumValue) ==
            roOptions.cend())
        {
            roOptions.push_back(enumValue);
        }
    };

    while (true)
    {
        int result = getopt(argc, argv, optstr.c_str());

        if (result == -1)
        {
            break;
        }

        switch (result)
        {
            case 'f':
                options.hex_file = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::HexFile);
                break;

            case '0':
                options.drives[0] = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::Drive0);
                break;

            case '1':
                options.drives[1] = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::Drive1);
                break;

            case '2':
                options.drives[2] = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::Drive2);
                break;

            case '3':
                options.drives[3] = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::Drive3);
                break;

            case 'p':
                options.disk_dir = fs::u8path(optarg);
                setReadOnly(FlexemuOptionId::DiskDirectory);
                break;

            case 'm':
                options.isHiMem = true;
                setReadOnly(FlexemuOptionId::IsRamExt2x288);
                break;

            case 'u':
                options.use_undocumented = true;
                setReadOnly(FlexemuOptionId::IsUseUndocumented);
                break;

            case 'j':
                {
                    std::stringstream str(optarg);

                    if (!(str >> i) || i < 1 || i > SCREEN_SIZES)
                    {
                        std::cerr << "Invalid -j value: '" << optarg << "'.\n"
                            "Only values 1 to " << SCREEN_SIZES <<
                            " are allowed.\n";
                        exit(EXIT_FAILURE);
                    }
                }

                options.pixelSize = i;
                setReadOnly(FlexemuOptionId::PixelSize);

                break;

            case 'F':
                {
                    std::stringstream str(optarg);

                    if (!(str >> f) || (f < 0.0 && f != -1.0))
                    {
                        std::cerr << "Invalid -F value: '" << optarg << "'.\n"
                            "Only values >= 0.0 or -1.0 are allowed, "
                            "unit is MHz.\n";
                        exit(EXIT_FAILURE);
                    }
                }

                options.frequency = f;
                setReadOnly(FlexemuOptionId::Frequency);

                break;

            case 'C':
                options.startup_command = optarg;
                break;
#ifdef UNIX
#ifdef HAVE_TERMIOS_H

            case 't':
                options.term_mode = true;
                break;

            case 'r':
                {
                    std::stringstream str(optarg);

                    if (std::strlen(optarg) != 2 ||
                        !(str >> std::hex >> options.reset_key))
                    {
                        std::cerr << "Invalid -r value: '" << optarg << "'.\n"
                            "Only a two digit hex value is allowed.\n";
                        exit(EXIT_FAILURE);
                    }
                }
                break;
#endif
            case 'T':
                {
                    std::string str(optarg);

                    if (str.compare("scroll") != 0 &&
                        str.compare("curses") != 0)
                    {
                        std::cerr << "Invalid -T value: '" << optarg << "'.\n"
                            "Only 'scroll' and 'curses' is supported.\n";
                        exit(EXIT_FAILURE);
                    }

                    if (str.compare("scroll") == 0)
                    {
                        options.terminalType = 1;
                    }
                    else if (str.compare("curses") == 0)
                    {
                        options.terminalType = 2;
                    }
                    setReadOnly(FlexemuOptionId::TerminalType);
                }
                break;
#endif
            case 'n':
                {
                    std::stringstream str(optarg);

                    if (!(str >> i) ||
                        (i != 2 && i != 8 && i != 64))
                    {
                        std::cerr << "Invalid -n value: '" << optarg << "'.\n"
                            "Only 2, 8 or 64 colors are allowed.\n";
                        exit(EXIT_FAILURE);
                    }
                }
                options.nColors = i;
                setReadOnly(FlexemuOptionId::NColors);
                break;

            case 'c':
                options.color = optarg;
                setReadOnly(FlexemuOptionId::Color);
                break;

            case 'i':
                options.isInverse = true;
                setReadOnly(FlexemuOptionId::IsInverse);
                break;

            case 'O':
                if (std::string(optarg).size() != 4)
                {
                    std::cerr << "Invalid -O value: '" << optarg << "'.\n"
                        "It should have four characters, each "
                        "can be '0', '1' or '-'.\n";
                    exit(EXIT_FAILURE);
                }
                i = 0;
                for (auto ch : std::string(optarg))
                {
                    if (ch != '0' && ch != '1' && ch != '-')
                    {
                        std::cerr << "parameter -O contains wrong value '" <<
                            ch << "' for drive number " << i << ".\n";
                        exit(EXIT_FAILURE);
                    }
                    if (ch != '-')
                    {
                        options.canFormatDrives[i] = (ch == '1');
                        setReadOnly(canFormatDriveOptionId[i]);
                    }
                    ++i;
                }
                break;

            case 'L':
                {
                    const auto tmp = fs::u8path(optarg);
                    const auto ext = flx::tolower(tmp.extension().u8string());
                    if (ext != ".log" && ext != ".txt" && ext != ".csv")
                    {
                        std::cerr << "logging path '" <<
                            tmp << "' has an unsupported file extension.\n";
                        exit(EXIT_FAILURE);
                    }
                    options.cpuLogPath = tmp;
                }
                break;

            case 'V':
                flx::print_versions(std::cout, PROGRAMNAME);
                exit(EXIT_SUCCESS);

            case '?':
            case 'h':
                PrintHelp(std::cerr);
                exit(EXIT_SUCCESS);
        }
    }
}


void FlexemuOptions::WriteOptions(
    const struct sOptions &options,
    bool ifNotExists, /* = false */
    bool isReadWriteOptionsOnly /* = false */
)
{
    auto optionIds(GetAllFlexemuOptionIds());

    if (isReadWriteOptionsOnly)
    {
        for (auto optionId : options.readOnlyOptionIds)
        {
            optionIds.erase(
                std::remove(optionIds.begin(), optionIds.end(), optionId),
                optionIds.end());
        }
    }
#ifdef _WIN32
    WriteOptionsToRegistry(options, optionIds, ifNotExists);
#else
    const auto rcFilePath = flx::getFlexemuUserConfigPath() / FLEXEMURC;
    fs::create_directories(flx::getFlexemuUserConfigPath());

    WriteOptionsToFile(options, optionIds, rcFilePath, ifNotExists);

    const auto oldRcFilePath = flx::getHomeDirectory() / OLDFLEXEMURC;
    if (fs::exists(oldRcFilePath))
    {
        fs::remove(oldRcFilePath);
    }
#endif
}

#ifdef _WIN32
void FlexemuOptions::WriteOptionsToRegistry(
        const struct sOptions &options,
        const std::vector<FlexemuOptionId> &optionIdsToWrite,
        bool ifNotExists /* = false */
        )
{
    BRegistry reg(BRegistry::currentUser, FLEXEMUREG);
    std::string str;
    bool ok;

    if (ifNotExists && reg.GetValue(FLEXVERSION, str) == ERROR_SUCCESS)
    {
        return;
    }

    // Only write options contained in optionIds.
    for (auto optionId : optionIdsToWrite)
    {
        switch (optionId)
        {
        case FlexemuOptionId::IsInverse:
            reg.SetValue(FLEXINVERSE, options.isInverse ? 1 : 0);
            break;

        case FlexemuOptionId::IsRamExt2x96:
            reg.SetValue(FLEXRAMEXTENSION, options.isRamExtension ? 1 : 0);
            break;

        case FlexemuOptionId::IsRamExt2x288:
            reg.SetValue(FLEXHIMEM, options.isHiMem ? 1 : 0);
            break;

        case FlexemuOptionId::IsFlexibleMmu:
            reg.SetValue(FLEXFLEXIBLEMMU, options.isFlexibleMmu ? 1 : 0);
            break;

        case FlexemuOptionId::IsEurocom2V5:
            reg.SetValue(FLEXEUROCOM2V5, options.isEurocom2V5 ? 1 : 0);
            break;

        case FlexemuOptionId::IsUseUndocumented:
            reg.SetValue(FLEXUNDOCUMENTED, options.use_undocumented ? 1 : 0);
            break;

        case FlexemuOptionId::IsUseRtc:
            reg.SetValue(FLEXRTC, options.useRtc ? 1 : 0);
            break;

        case FlexemuOptionId::Color:
            reg.SetValue(FLEXCOLOR, options.color.c_str());
            break;

        case FlexemuOptionId::NColors:
            reg.SetValue(FLEXNCOLORS, options.nColors);
            break;

        case FlexemuOptionId::PixelSize:
            reg.SetValue(FLEXSCREENFACTOR, options.pixelSize);
            break;

        case FlexemuOptionId::IconSize:
            reg.SetValue(FLEXICONSIZE, options.iconSize);
            break;

        case FlexemuOptionId::HexFile:
            reg.SetValue(FLEXMONITOR, options.hex_file.u8string());
            break;

        case FlexemuOptionId::DiskDirectory:
            reg.SetValue(FLEXDISKDIR, options.disk_dir.u8string());
            break;

        case FlexemuOptionId::Drive0:
            reg.SetValue(FLEXDISK0, options.drives[0].u8string());
            break;

        case FlexemuOptionId::Drive1:
            reg.SetValue(FLEXDISK1, options.drives[1].u8string());
            break;

        case FlexemuOptionId::Drive2:
            reg.SetValue(FLEXDISK2, options.drives[2].u8string());
            break;

        case FlexemuOptionId::Drive3:
            reg.SetValue(FLEXDISK3, options.drives[3].u8string());
            break;

        case FlexemuOptionId::MdcrDrive0:
            reg.SetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0].u8string());
            break;

        case FlexemuOptionId::MdcrDrive1:
            reg.SetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1].u8string());
            break;

        case FlexemuOptionId::CanFormatDrive0:
            reg.SetValue(FLEXFORMATDRIVE0, options.canFormatDrives[0] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive1:
            reg.SetValue(FLEXFORMATDRIVE1, options.canFormatDrives[1] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive2:
            reg.SetValue(FLEXFORMATDRIVE2, options.canFormatDrives[2] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive3:
            reg.SetValue(FLEXFORMATDRIVE3, options.canFormatDrives[3] ? 1 : 0);
            break;

        case FlexemuOptionId::Frequency:
            str =
              flx::toString<decltype(options.frequency)>(options.frequency, ok);
            if (ok)
            {
                reg.SetValue(FLEXFREQUENCY, str);
            }
            break;

        case FlexemuOptionId::FileTimeAccess:
            reg.SetValue(FLEXFILETIMEACCESS,
                static_cast<int>(options.fileTimeAccess));
            break;

        case FlexemuOptionId::IsDisplaySmooth:
            reg.SetValue(FLEXDISPLAYSMOOTH, options.isSmooth ? 1 : 0);
            break;

        case FlexemuOptionId::IsConfirmExit:
            reg.SetValue(FLEXISCONFIRMEXIT, options.isConfirmExit ? 1 : 0);
            break;

        case FlexemuOptionId::DirectoryDiskTrkSec:
            reg.SetValue(FLEXDIRECTORYDISKTRACKS,
                options.directoryDiskTracks);
            reg.SetValue(FLEXDIRECTORYDISKSECTORS,
                options.directoryDiskSectors);
            break;

        case FlexemuOptionId::PrintFont:
            reg.SetValue(FLEXPRINTFONT, options.printFont.c_str());
            break;

        case FlexemuOptionId::IsPrintPageBreakDetected:
            reg.SetValue(FLEXPRINTPAGEBREAKDETECTED,
                    options.isPrintPageBreakDetected ? 1 : 0);
            break;

        case FlexemuOptionId::PrintOrientation:
            reg.SetValue(FLEXPRINTORIENTATION,
                    options.printOrientation.c_str());
            break;

        case FlexemuOptionId::PrintPageSize:
            reg.SetValue(FLEXPRINTPAGESIZE, options.printPageSize.c_str());
            break;

        case FlexemuOptionId::PrintUnit:
            reg.SetValue(FLEXPRINTUNIT, options.printUnit.c_str());
            break;

        case FlexemuOptionId::PrintOutputWindowGeometry:
            reg.SetValue(FLEXPRINTOUTPUTWINDOWGEOMETRY,
                    options.printOutputWindowGeometry.c_str());
            break;

        case FlexemuOptionId::PrintPreviewDialogGeometry:
            reg.SetValue(FLEXPRINTPREVIEWDIALOGGEOMETRY,
                    options.printPreviewDialogGeometry.c_str());
            break;

        case FlexemuOptionId::PrintConfigs:
            for (const auto& [subKey, value] : options.printConfigs)
            {
                const auto key = std::string(FLEXPRINTCONFIG) + subKey;
                reg.SetValue(key.c_str(), value.c_str());
            }
            break;

        case FlexemuOptionId::IsStatusBarVisible:
            reg.SetValue(FLEXISSTATUSBARVISIBLE,
                options.isStatusBarVisible ? 1 : 0);
                break;

        case FlexemuOptionId::IsDirectoryDiskActive:
            reg.SetValue(FLEXISDIRECTORYDISKACTIVE,
                options.isDirectoryDiskActive ? 1 : 0);
        }
    }
}
#else
void FlexemuOptions::WriteOptionsToFile(
        const struct sOptions &options,
        const std::vector<FlexemuOptionId> &optionIdsToWrite,
        const fs::path &path,
        bool ifNotExists /* = false */)
{
    if (ifNotExists && fs::exists(path))
    {
        return;
    }

    auto optionIds(GetAllFlexemuOptionIds());

    // Collect all optionIds to overwrite from previous options.
    for (auto optionId : optionIdsToWrite)
    {
        optionIds.erase(
            std::remove(optionIds.begin(), optionIds.end(), optionId),
            optionIds.end());
    }

    sOptions previousOptions; // Previous options read from file.
    sOptions optionsToWrite(options); // Options to write to file.

    // Read previous options from file.
    InitOptions(previousOptions);
    GetOptions(previousOptions);

    for (auto optionId : optionIds)
    {
        switch (optionId)
        {
        case FlexemuOptionId::IsInverse:
            optionsToWrite.isInverse = previousOptions.isInverse;
            break;

        case FlexemuOptionId::IsRamExt2x96:
            optionsToWrite.isRamExtension = previousOptions.isRamExtension;
            break;

        case FlexemuOptionId::IsRamExt2x288:
            optionsToWrite.isHiMem = previousOptions.isHiMem;
            break;

        case FlexemuOptionId::IsFlexibleMmu:
            optionsToWrite.isFlexibleMmu = previousOptions.isFlexibleMmu;
            break;

        case FlexemuOptionId::IsEurocom2V5:
            optionsToWrite.isEurocom2V5 = previousOptions.isEurocom2V5;
            break;

        case FlexemuOptionId::IsUseUndocumented:
            optionsToWrite.use_undocumented = previousOptions.use_undocumented;
            break;

        case FlexemuOptionId::IsUseRtc:
            optionsToWrite.useRtc = previousOptions.useRtc;
            break;

        case FlexemuOptionId::Color:
            optionsToWrite.color = previousOptions.color;
            break;

        case FlexemuOptionId::NColors:
            optionsToWrite.nColors = previousOptions.nColors;
            break;

        case FlexemuOptionId::PixelSize:
            optionsToWrite.pixelSize = previousOptions.pixelSize;
            break;

        case FlexemuOptionId::IconSize:
            optionsToWrite.iconSize = previousOptions.iconSize;
            break;

        case FlexemuOptionId::HexFile:
            optionsToWrite.hex_file = previousOptions.hex_file;
            break;

        case FlexemuOptionId::DiskDirectory:
            optionsToWrite.disk_dir = previousOptions.disk_dir;
            break;

        case FlexemuOptionId::Drive0:
            optionsToWrite.drives[0] = previousOptions.drives[0];
            break;

        case FlexemuOptionId::Drive1:
            optionsToWrite.drives[1] = previousOptions.drives[1];
            break;

        case FlexemuOptionId::Drive2:
            optionsToWrite.drives[2] = previousOptions.drives[2];
            break;

        case FlexemuOptionId::Drive3:
            optionsToWrite.drives[3] = previousOptions.drives[3];
            break;

        case FlexemuOptionId::CanFormatDrive0:
            optionsToWrite.canFormatDrives[0] =
                previousOptions.canFormatDrives[0];
            break;

        case FlexemuOptionId::CanFormatDrive1:
            optionsToWrite.canFormatDrives[1] =
                previousOptions.canFormatDrives[1];
            break;

        case FlexemuOptionId::CanFormatDrive2:
            optionsToWrite.canFormatDrives[2] =
                previousOptions.canFormatDrives[2];
            break;

        case FlexemuOptionId::CanFormatDrive3:
            optionsToWrite.canFormatDrives[3] =
                previousOptions.canFormatDrives[3];
            break;

        case FlexemuOptionId::MdcrDrive0:
            optionsToWrite.mdcrDrives[0] = previousOptions.mdcrDrives[0];
            break;

        case FlexemuOptionId::MdcrDrive1:
            optionsToWrite.mdcrDrives[1] = previousOptions.mdcrDrives[1];
            break;

        case FlexemuOptionId::Frequency:
            optionsToWrite.frequency = previousOptions.frequency;
            break;

        case FlexemuOptionId::FileTimeAccess:
            optionsToWrite.fileTimeAccess = previousOptions.fileTimeAccess;
            break;

        case FlexemuOptionId::IsDisplaySmooth:
            optionsToWrite.isSmooth = previousOptions.isSmooth;
            break;

        case FlexemuOptionId::IsConfirmExit:
            optionsToWrite.isConfirmExit = previousOptions.isConfirmExit;
            break;

        case FlexemuOptionId::IsTerminalIgnoreESC:
            optionsToWrite.isTerminalIgnoreESC =
                previousOptions.isTerminalIgnoreESC;
            break;

        case FlexemuOptionId::IsTerminalIgnoreNUL:
            optionsToWrite.isTerminalIgnoreNUL =
                previousOptions.isTerminalIgnoreNUL;
            break;

        case FlexemuOptionId::TerminalType:
            optionsToWrite.terminalType = previousOptions.terminalType;
            break;

        case FlexemuOptionId::IsDirectoryDiskActive:
            optionsToWrite.isDirectoryDiskActive =
                previousOptions.isDirectoryDiskActive;
            break;

        case FlexemuOptionId::DirectoryDiskTrkSec:
            optionsToWrite.directoryDiskTracks =
                previousOptions.directoryDiskTracks;
            optionsToWrite.directoryDiskSectors =
                previousOptions.directoryDiskSectors;
            break;

        case FlexemuOptionId::PrintFont:
            optionsToWrite.printFont = previousOptions.printFont;
            break;

        case FlexemuOptionId::IsPrintPageBreakDetected:
            optionsToWrite.isPrintPageBreakDetected =
                previousOptions.isPrintPageBreakDetected;
            break;

        case FlexemuOptionId::PrintOrientation:
            optionsToWrite.printOrientation = previousOptions.printOrientation;
            break;

        case FlexemuOptionId::PrintPageSize:
            optionsToWrite.printPageSize = previousOptions.printPageSize;
            break;

        case FlexemuOptionId::PrintUnit:
            optionsToWrite.printUnit = previousOptions.printUnit;
            break;

        case FlexemuOptionId::PrintOutputWindowGeometry:
            optionsToWrite.printOutputWindowGeometry =
                    previousOptions.printOutputWindowGeometry;
            break;

        case FlexemuOptionId::PrintPreviewDialogGeometry:
            optionsToWrite.printPreviewDialogGeometry =
                    previousOptions.printPreviewDialogGeometry;
            break;

        case FlexemuOptionId::PrintConfigs:
            optionsToWrite.printConfigs = previousOptions.printConfigs;
            break;

        case FlexemuOptionId::IsStatusBarVisible:
            optionsToWrite.isStatusBarVisible =
                previousOptions.isStatusBarVisible;
            break;
        }

        optionsToWrite.version = VERSION;
    }

    BRcFile rcFile(path);
    bool ok;
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXVERSION, VERSION);
    rcFile.SetValue(FLEXINVERSE, optionsToWrite.isInverse ? 1 : 0);
    rcFile.SetValue(FLEXDISPLAYSMOOTH, optionsToWrite.isSmooth ? 1 : 0);
    rcFile.SetValue(FLEXISCONFIRMEXIT, optionsToWrite.isConfirmExit ? 1 : 0);
    rcFile.SetValue(FLEXCOLOR, optionsToWrite.color);
    rcFile.SetValue(FLEXNCOLORS, optionsToWrite.nColors);
    rcFile.SetValue(FLEXSCREENFACTOR, optionsToWrite.pixelSize);
    rcFile.SetValue(FLEXICONSIZE, optionsToWrite.iconSize);
    rcFile.SetValue(FLEXMONITOR, optionsToWrite.hex_file);
    rcFile.SetValue(FLEXDISKDIR, optionsToWrite.disk_dir);
    rcFile.SetValue(FLEXDISK0, optionsToWrite.drives[0]);
    rcFile.SetValue(FLEXDISK1, optionsToWrite.drives[1]);
    rcFile.SetValue(FLEXDISK2, optionsToWrite.drives[2]);
    rcFile.SetValue(FLEXDISK3, optionsToWrite.drives[3]);
    rcFile.SetValue(FLEXMDCRDRIVE0, optionsToWrite.mdcrDrives[0]);
    rcFile.SetValue(FLEXMDCRDRIVE1, optionsToWrite.mdcrDrives[1]);
    rcFile.SetValue(FLEXRAMEXTENSION, optionsToWrite.isRamExtension ? 1 : 0);
    rcFile.SetValue(FLEXHIMEM, optionsToWrite.isHiMem ? 1 : 0);
    rcFile.SetValue(FLEXFLEXIBLEMMU, optionsToWrite.isFlexibleMmu ? 1 : 0);
    rcFile.SetValue(FLEXEUROCOM2V5, optionsToWrite.isEurocom2V5 ? 1 : 0);
    rcFile.SetValue(FLEXUNDOCUMENTED, optionsToWrite.use_undocumented ? 1 : 0);
    rcFile.SetValue(FLEXRTC, optionsToWrite.useRtc ? 1 : 0);
    auto str = flx::toString<decltype(options.frequency)>(
                optionsToWrite.frequency, ok);
    if (ok)
    {
        rcFile.SetValue(FLEXFREQUENCY, str);
    }
    rcFile.SetValue(FLEXFORMATDRIVE0,
            optionsToWrite.canFormatDrives[0] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE1,
            optionsToWrite.canFormatDrives[1] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE2,
            optionsToWrite.canFormatDrives[2] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE3,
            optionsToWrite.canFormatDrives[3] ? 1 : 0);
    rcFile.SetValue(FLEXFILETIMEACCESS,
                    static_cast<int>(optionsToWrite.fileTimeAccess));
    rcFile.SetValue(FLEXTERMINALIGNOREESC,
            optionsToWrite.isTerminalIgnoreESC ? 1 : 0);
    rcFile.SetValue(FLEXTERMINALIGNORENUL,
            optionsToWrite.isTerminalIgnoreNUL ? 1 : 0);
    rcFile.SetValue(FLEXTERMINALTYPE, optionsToWrite.terminalType);
    rcFile.SetValue(FLEXDIRECTORYDISKTRACKS,
            optionsToWrite.directoryDiskTracks);
    rcFile.SetValue(FLEXDIRECTORYDISKSECTORS,
            optionsToWrite.directoryDiskSectors);
    rcFile.SetValue(FLEXISDIRECTORYDISKACTIVE,
            optionsToWrite.isDirectoryDiskActive ? 1 : 0);
    rcFile.SetValue(FLEXISSTATUSBARVISIBLE,
            optionsToWrite.isStatusBarVisible ? 1 : 0);
    rcFile.SetValue(FLEXPRINTFONT, optionsToWrite.printFont);
    rcFile.SetValue(FLEXPRINTPAGEBREAKDETECTED,
            optionsToWrite.isPrintPageBreakDetected ? 1 : 0);
    rcFile.SetValue(FLEXPRINTORIENTATION, optionsToWrite.printOrientation);
    rcFile.SetValue(FLEXPRINTPAGESIZE, optionsToWrite.printPageSize);
    rcFile.SetValue(FLEXPRINTUNIT, optionsToWrite.printUnit);
    rcFile.SetValue(FLEXPRINTOUTPUTWINDOWGEOMETRY,
            optionsToWrite.printOutputWindowGeometry);
    rcFile.SetValue(FLEXPRINTPREVIEWDIALOGGEOMETRY,
            optionsToWrite.printPreviewDialogGeometry);

    for (const auto &iter : optionsToWrite.printConfigs)
    {
        const auto key = std::string(FLEXPRINTCONFIG) + iter.first;
        rcFile.SetValue(key.c_str(), iter.second);
    }
}
#endif

void FlexemuOptions::GetOptions(struct sOptions &options)
{
    int int_result;
    std::string string_result;

#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXEMUREG);

    reg.GetValue(FLEXVERSION, options.version);
    reg.GetValue(FLEXDISKDIR, string_result);
    options.disk_dir = fs::u8path(string_result);
    reg.GetValue(FLEXDISK0, string_result);
    options.drives[0] = fs::u8path(string_result);
    reg.GetValue(FLEXDISK1, string_result);
    options.drives[1] = fs::u8path(string_result);
    reg.GetValue(FLEXDISK2, string_result);
    options.drives[2] = fs::u8path(string_result);
    reg.GetValue(FLEXDISK3, string_result);
    options.drives[3] = fs::u8path(string_result);
    reg.GetValue(FLEXMDCRDRIVE0, string_result);
    options.mdcrDrives[0] = fs::u8path(string_result);
    reg.GetValue(FLEXMDCRDRIVE1, string_result);
    options.mdcrDrives[1] = fs::u8path(string_result);
    reg.GetValue(FLEXMONITOR, string_result);
    options.hex_file = fs::u8path(string_result);
    reg.GetValue(FLEXCOLOR, options.color);

    if (!reg.GetValue(FLEXNCOLORS, int_result))
    {
        if (int_result == 2 || int_result == 8 || int_result == 64)
        {
            options.nColors = int_result;
        }
    }

    if (!reg.GetValue(FLEXSCREENFACTOR, int_result))
    {
        if (int_result < 1)
        {
            int_result = 1;
        }

        if (int_result > SCREEN_SIZES)
        {
            int_result = SCREEN_SIZES;
        }

        options.pixelSize = int_result;
    }

    if (!reg.GetValue(FLEXICONSIZE, int_result))
    {
        options.iconSize = (int_result >= 24) ? 24 : 16;
        options.iconSize = (int_result >= 32) ? 32 : options.iconSize;
    }

    if (!reg.GetValue(FLEXINVERSE, int_result))
    {
        options.isInverse = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRAMEXTENSION, int_result))
    {
        options.isRamExtension = (int_result != 0);
    }

    if (!reg.GetValue(FLEXHIMEM, int_result))
    {
        options.isHiMem = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFLEXIBLEMMU, int_result))
    {
        options.isFlexibleMmu = (int_result != 0);
    }

    if (!reg.GetValue(FLEXEUROCOM2V5, int_result))
    {
        options.isEurocom2V5 = (int_result != 0);
    }

    if (!reg.GetValue(FLEXUNDOCUMENTED, int_result))
    {
        options.use_undocumented = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!reg.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFREQUENCY, string_result))
    {
        using T = decltype(options.frequency);
        T value{};

        if (flx::fromString<T>(string_result, value))
        {
            options.frequency = value;
        }
    }

    if (!reg.GetValue(FLEXFORMATDRIVE0, int_result))
    {
        options.canFormatDrives[0] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE1, int_result))
    {
        options.canFormatDrives[1] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE2, int_result))
    {
        options.canFormatDrives[2] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE3, int_result))
    {
        options.canFormatDrives[3] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFILETIMEACCESS, int_result))
    {
        if (int_result < 0)
        {
            int_result = 0;
        }
        else if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.fileTimeAccess = static_cast<FileTimeAccess>(int_result);
    }

    if (!reg.GetValue(FLEXDISPLAYSMOOTH, int_result))
    {
        options.isSmooth = (int_result != 0);
    }

    if (!reg.GetValue(FLEXISCONFIRMEXIT, int_result))
    {
        options.isConfirmExit = (int_result != 0);
    }

    if (!reg.GetValue(FLEXDIRECTORYDISKTRACKS, int_result))
    {
        int_result = std::max(int_result, 2);
        int_result = std::min(int_result, 256);
        options.directoryDiskTracks = int_result;
    }

    if (!reg.GetValue(FLEXDIRECTORYDISKSECTORS, int_result))
    {
        int_result = std::max(int_result, 6);
        int_result = std::min(int_result, 255);
        options.directoryDiskSectors = int_result;
    }

    if (!reg.GetValue(FLEXISDIRECTORYDISKACTIVE, int_result))
    {
        options.isDirectoryDiskActive = (int_result != 0);
    }

    if (!reg.GetValue(FLEXISSTATUSBARVISIBLE, int_result))
    {
        options.isStatusBarVisible = (int_result != 0);
    }

    reg.GetValue(FLEXPRINTFONT, options.printFont);

    if (!reg.GetValue(FLEXPRINTPAGEBREAKDETECTED, int_result))
    {
        options.isPrintPageBreakDetected = (int_result != 0);
    }

    reg.GetValue(FLEXPRINTORIENTATION, options.printOrientation);
    reg.GetValue(FLEXPRINTPAGESIZE, options.printPageSize);
    reg.GetValue(FLEXPRINTUNIT, options.printUnit);
    reg.GetValue(FLEXPRINTOUTPUTWINDOWGEOMETRY,
            options.printOutputWindowGeometry);
    reg.GetValue(FLEXPRINTPREVIEWDIALOGGEOMETRY,
            options.printPreviewDialogGeometry);
    reg.GetValues(FLEXPRINTCONFIG, options.printConfigs);
#endif
#ifdef UNIX
    auto rcFilePath = flx::getFlexemuUserConfigPath() / FLEXEMURC;

    if (!fs::exists(rcFilePath))
    {
        rcFilePath = flx::getHomeDirectory() / OLDFLEXEMURC;
    }

    BRcFile rcFile(rcFilePath);
    rcFile.GetValue(FLEXVERSION, options.version);
    rcFile.GetValue(FLEXDISKDIR, string_result);
    options.disk_dir = string_result;
    rcFile.GetValue(FLEXDISK0, string_result);
    options.drives[0] = string_result;
    rcFile.GetValue(FLEXDISK1, string_result);
    options.drives[1] = string_result;
    rcFile.GetValue(FLEXDISK2, string_result);
    options.drives[2] = string_result;
    rcFile.GetValue(FLEXDISK3, string_result);
    options.drives[3] = string_result;
    rcFile.GetValue(FLEXMDCRDRIVE0, string_result);
    options.mdcrDrives[0] = string_result;
    rcFile.GetValue(FLEXMDCRDRIVE1, string_result);
    options.mdcrDrives[1] = string_result;
    rcFile.GetValue(FLEXMONITOR, string_result);
    options.hex_file = string_result;
    rcFile.GetValue(FLEXCOLOR, options.color);

    if (!rcFile.GetValue(FLEXNCOLORS, int_result))
    {
        if (int_result == 2 || int_result == 8 || int_result == 64)
        {
            options.nColors = int_result;
        }
    }

    if (!rcFile.GetValue(FLEXSCREENFACTOR, int_result))
    {
        int_result = std::max(int_result, 1);
        int_result = std::min<int>(int_result, SCREEN_SIZES);

        options.pixelSize = int_result;
    }

    if (!rcFile.GetValue(FLEXICONSIZE, int_result))
    {
        options.iconSize = (int_result >= 24) ? 24 : 16;
        options.iconSize = (int_result >= 32) ? 32 : options.iconSize;
    }

    if (!rcFile.GetValue(FLEXINVERSE, int_result))
    {
        options.isInverse = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXRAMEXTENSION, int_result))
    {
        options.isRamExtension = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXHIMEM, int_result))
    {
        options.isHiMem = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFLEXIBLEMMU, int_result))
    {
        options.isFlexibleMmu = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXEUROCOM2V5, int_result))
    {
        options.isEurocom2V5 = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXUNDOCUMENTED, int_result))
    {
        options.use_undocumented = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXRTC, int_result))
    {
        options.useRtc = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFREQUENCY, string_result))
    {
        using T = decltype(options.frequency);
        T value{};
        if (flx::fromString<T>(string_result, value))
        {
            options.frequency = value;
        }
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE0, int_result))
    {
        options.canFormatDrives[0] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE1, int_result))
    {
        options.canFormatDrives[1] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE2, int_result))
    {
        options.canFormatDrives[2] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE3, int_result))
    {
        options.canFormatDrives[3] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFILETIMEACCESS, int_result))
    {
        if (int_result < 0)
        {
            int_result = 0;
        }
        else if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.fileTimeAccess = static_cast<FileTimeAccess>(int_result);
    }

    if (!rcFile.GetValue(FLEXDISPLAYSMOOTH, int_result))
    {
        options.isSmooth = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXISCONFIRMEXIT, int_result))
    {
        options.isConfirmExit = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXTERMINALIGNOREESC, int_result))
    {
        options.isTerminalIgnoreESC = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXTERMINALIGNORENUL, int_result))
    {
        options.isTerminalIgnoreNUL = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXTERMINALTYPE, int_result))
    {
        int_result = std::max(int_result, 1);
        int_result = std::min(int_result, 2);
        options.terminalType = int_result;
    }

    if (!rcFile.GetValue(FLEXDIRECTORYDISKTRACKS, int_result))
    {
        int_result = std::max(int_result, 2);
        int_result = std::min(int_result, 256);
        options.directoryDiskTracks = int_result;
    }

    if (!rcFile.GetValue(FLEXDIRECTORYDISKSECTORS, int_result))
    {
        int_result = std::max(int_result, 6);
        int_result = std::min(int_result, 255);
        options.directoryDiskSectors = int_result;
    }

    if (!rcFile.GetValue(FLEXISDIRECTORYDISKACTIVE, int_result))
    {
        options.isDirectoryDiskActive = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXISSTATUSBARVISIBLE, int_result))
    {
        options.isStatusBarVisible = (int_result != 0);
    }

    rcFile.GetValue(FLEXPRINTFONT, options.printFont);

    if (!rcFile.GetValue(FLEXPRINTPAGEBREAKDETECTED, int_result))
    {
        options.isPrintPageBreakDetected = (int_result != 0);
    }

    rcFile.GetValue(FLEXPRINTORIENTATION, options.printOrientation);
    rcFile.GetValue(FLEXPRINTPAGESIZE, options.printPageSize);
    rcFile.GetValue(FLEXPRINTUNIT, options.printUnit);
    rcFile.GetValue(FLEXPRINTOUTPUTWINDOWGEOMETRY,
            options.printOutputWindowGeometry);
    rcFile.GetValue(FLEXPRINTPREVIEWDIALOGGEOMETRY,
            options.printPreviewDialogGeometry);
    rcFile.GetValues(FLEXPRINTCONFIG, options.printConfigs);
#endif
}

