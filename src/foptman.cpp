/*
    foptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include <signal.h>
#include <stdexcept>
#include <string>
#include <new>

#include "e2.h"
#include "flexemu.h"
#include "soptions.h"
#include "foptman.h"
#include "bregistr.h"
#include "brcfile.h"


const char * const FLEXEMURC = ".flexemurc";

const char * const FLEXDISKDIR = "DiskDirectory";
const char * const FLEXDISK0 = "Disk0Path";
const char * const FLEXDISK1 = "Disk1Path";
const char * const FLEXDISK2 = "Disk2Path";
const char * const FLEXDISK3 = "Disk3Path";
const char * const FLEXMDCRDRIVE0 = "MdcrDrive0Path";
const char * const FLEXMDCRDRIVE1 = "MdcrDrive1Path";
const char * const FLEXCOLOR = "DisplayColor";
const char * const FLEXNCOLORS = "NoOfColors";
const char * const FLEXINVERSE = "DisplayInverse";
const char * const FLEXHIMEM = "HighMemory";
const char * const FLEXFLEXIBLEMMU = "UseFlexibleMmu";
const char * const FLEXRAMEXTENSION = "UseRamExtension";
const char * const FLEXEUROCOM2V5 = "UseEurocom2V5";
const char * const FLEXUNDOCUMENTED = "UndocumentedMc6809";
const char * const FLEXRTC = "UseRTC";
const char * const FLEXFORMATDRIVE0 = "CanFormatDrive0";
const char * const FLEXFORMATDRIVE1 = "CanFormatDrive1";
const char * const FLEXFORMATDRIVE2 = "CanFormatDrive2";
const char * const FLEXFORMATDRIVE3 = "CanFormatDrive3";
// variable used on WIN32 only.
// NOLINTNEXTLINE(clang-diagnostic-unused-const-variable)
const char * const FLEXDOCDIR = "DocDirectory";
const char * const FLEXMONITOR = "MonitorPath";
const char * const FLEXVERSION = "Version";
const char * const FLEXSCREENFACTOR = "ScreenFactor";
const char * const FLEXFREQUENCY = "Frequency";
const char * const FLEXFILETIMEACCESS = "FileTimeAccess";
const char * const FLEXDISPLAYSMOOTH = "DisplaySmooth";
const char * const FLEXTERMINALIGNOREESC = "TerminalIgnoreESC";
const char * const FLEXTERMINALIGNORENUL = "TerminalIgnoreNUL";
const char * const FLEXPRINTFONT = "PrintFont";
const char * const FLEXPRINTPAGEBREAKDETECTED = "PrintPageBreakDetected";
const char * const FLEXPRINTORIENTATION = "PrintOrientation";
const char * const FLEXPRINTPAGESIZE = "PrintPageSize";
const char * const FLEXPRINTUNIT = "PrintUnit";
const char * const FLEXPRINTOUTPUTWINDOWGEOMETRY = "PrintOutputWindowGeometry";
const char * const FLEXPRINTPREVIEWDIALOGGEOMETRY =
                    "PrintPreviewDialogGeometry";
const char * const FLEXPRINTCONFIG = "PrintConfig";
const char * const FLEXDIRECTORYDISKTRACKS = "DirectoryDiskTracks";
const char * const FLEXDIRECTORYDISKSECTORS = "DirectoryDiskSectors";
const char * const FLEXISDIRECTORYDISKACTIVE = "IsDirectoryDiskActive";

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
#ifdef HAVE_TERMIOS_H
          "  -t (terminal only mode)\n"
          "  -r <two-hex-digit reset key>\n"
#endif
          "  -c <color> define foreground color\n"
          "  -i (display inverse video)\n"
          "  -O <cccc> (Support formatting disk in drive 0..3\n"
          "     'c' represents drive 0..3 and can be: '0'=no; '1'=yes or "
            "'-'=unchanged,\n"
          "     Example: -O 001- Drive 2 allows format, drive 0,1 not, "
          "drive 3 unchanged.\n"
          "  -n <# of colors>\n"
          "  -h (display this)\n"
          "  -? (display this)\n"
          "  -v (print version number)\n";
} // PrintHelp

void FlexemuOptions::InitOptions(struct sOptions &options)
{
    options.drive[0] = "system.dsk";
    options.drive[1] = "";
    options.drive[2] = "";
    options.drive[3] = "";
    options.mdcrDrives[0] = "system.mdcr";
    options.mdcrDrives[1] = "";
    options.hex_file = "neumon54.hex";
    options.startup_command = "";
    options.term_mode = false;
    options.isRamExtension = true;
    options.isHiMem = false;
    options.isFlexibleMmu = false;
    options.isEurocom2V5 = false;
    options.use_undocumented = false;
    options.useRtc = true;
    options.canFormatDrive[0] = false;
    options.canFormatDrive[1] = false;
    options.canFormatDrive[2] = false;
    options.canFormatDrive[3] = false;
    options.fileTimeAccess = FileTimeAccess::NONE;
    options.reset_key = 0x1e; // is Ctrl-^ for reset or Sig. INT
    options.frequency = -1.0; // default: ignore

    options.color = "green";
    options.nColors = 2;
    options.isInverse = false;
    options.isSmooth = false;
    options.isTerminalIgnoreESC = true;
    options.isTerminalIgnoreNUL = true;
#ifdef UNIX
    options.doc_dir = F_DATADIR;
    options.disk_dir = F_DATADIR;
#endif
#ifdef _WIN32
    options.doc_dir = getExecutablePath() + PATHSEPARATORSTRING + "Documentation";
    options.disk_dir = getExecutablePath() + PATHSEPARATORSTRING + "Data";
#endif
    options.pixelSize = 2;
    options.readOnlyOptionIds.clear();
    options.isPrintPageBreakDetected = false;
    options.directoryDiskTracks = 80;
    options.directoryDiskSectors = 36;
    options.isDirectoryDiskActive = true;
} // InitOptions

void FlexemuOptions::GetCommandlineOptions(
    struct sOptions &options,
    int argc,
    char *const argv[])
{
    int i;
    float f;
    optind = 1;
    opterr = 1;
    std::string optstr("mup:f:0:1:2:3:j:F:C:O:");
#ifdef HAVE_TERMIOS_H
    optstr.append("tr:"); // terminal mode and reset key
#endif
    optstr.append("ic:n:"); // color, inverse video, # of colors
    optstr.append("vh"); // version and help

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
                options.hex_file = optarg;
                setReadOnly(FlexemuOptionId::HexFile);
                break;

            case '0':
                options.drive[0] = optarg;
                setReadOnly(FlexemuOptionId::Drive0);
                break;

            case '1':
                options.drive[1] = optarg;
                setReadOnly(FlexemuOptionId::Drive1);
                break;

            case '2':
                options.drive[2] = optarg;
                setReadOnly(FlexemuOptionId::Drive2);
                break;

            case '3':
                options.drive[3] = optarg;
                setReadOnly(FlexemuOptionId::Drive3);
                break;

            case 'p':
                options.disk_dir = optarg;
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
#ifdef HAVE_TERMIOS_H

            case 't':
                options.term_mode = true;
                break;

            case 'r':
                {
                    std::stringstream str(optarg);

                    if (strlen(optarg) != 2 ||
                        !(str >> std::hex >> options.reset_key))
                    {
                        std::cerr << "Invalid -r value: '" << optarg << "'.\n"
                            "Only a two digit hex value is allowed.\n";
                        exit(EXIT_FAILURE);
                    }
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
                        options.canFormatDrive[i] = (ch == '1');
                        setReadOnly(canFormatDriveOptionId[i]);
                    }
                    ++i;
                }
                break;

            case 'v':
                std::cout << PROGRAMNAME ": V " PROGRAM_VERSION "\n";
                exit(EXIT_SUCCESS);

            case '?':
            case 'h':
                PrintHelp(std::cerr);
                exit(EXIT_SUCCESS);
        }  // switch
    } // while
} // GetCommandlineOptions


void FlexemuOptions::WriteOptions(
    const struct sOptions &options,
    bool ifNotExists, /* = false */
    bool isReadWriteOptionsOnly /* = false */
)
{
    auto optionIds(allFlexemuOptionIds);

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
#endif
#ifdef UNIX
    const auto rcFileName = (getHomeDirectory() += PATHSEPARATORSTRING) +=
        FLEXEMURC;

    WriteOptionsToFile(options, optionIds, rcFileName, ifNotExists);
#endif

} /* WriteOptions */

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

        case FlexemuOptionId::HexFile:
            reg.SetValue(FLEXMONITOR, options.hex_file.c_str());
            break;

        case FlexemuOptionId::DiskDirectory:
            reg.SetValue(FLEXDISKDIR, options.disk_dir.c_str());
            break;

        case FlexemuOptionId::Drive0:
            reg.SetValue(FLEXDISK0, options.drive[0].c_str());
            break;

        case FlexemuOptionId::Drive1:
            reg.SetValue(FLEXDISK1, options.drive[1].c_str());
            break;

        case FlexemuOptionId::Drive2:
            reg.SetValue(FLEXDISK2, options.drive[2].c_str());
            break;

        case FlexemuOptionId::Drive3:
            reg.SetValue(FLEXDISK3, options.drive[3].c_str());
            break;

        case FlexemuOptionId::MdcrDrive0:
            reg.SetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0].c_str());
            break;

        case FlexemuOptionId::MdcrDrive1:
            reg.SetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1].c_str());
            break;

        case FlexemuOptionId::CanFormatDrive0:
            reg.SetValue(FLEXFORMATDRIVE0, options.canFormatDrive[0] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive1:
            reg.SetValue(FLEXFORMATDRIVE1, options.canFormatDrive[1] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive2:
            reg.SetValue(FLEXFORMATDRIVE2, options.canFormatDrive[2] ? 1 : 0);
            break;

        case FlexemuOptionId::CanFormatDrive3:
            reg.SetValue(FLEXFORMATDRIVE3, options.canFormatDrive[3] ? 1 : 0);
            break;

        case FlexemuOptionId::Frequency:
            str = toString<decltype(options.frequency)>(options.frequency, ok);
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
        }

        reg.SetValue(FLEXISDIRECTORYDISKACTIVE,
            options.isDirectoryDiskActive ? 1 : 0);

        reg.SetValue(FLEXVERSION, VERSION);
        reg.DeleteValue(FLEXDOCDIR); // Deprecated option value
    }
}
#endif

#ifdef UNIX
void FlexemuOptions::WriteOptionsToFile(
        const struct sOptions &options,
        const std::vector<FlexemuOptionId> &optionIdsToWrite,
        const std::string &fileName,
        bool ifNotExists /* = false */)
{
    if (ifNotExists && access(fileName.c_str(), F_OK) == 0)
    {
        return;
    }

    auto optionIds(allFlexemuOptionIds);

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

        case FlexemuOptionId::HexFile:
            optionsToWrite.hex_file = previousOptions.hex_file;
            break;

        case FlexemuOptionId::DiskDirectory:
            optionsToWrite.disk_dir = previousOptions.disk_dir;
            break;

        case FlexemuOptionId::Drive0:
            optionsToWrite.drive[0] = previousOptions.drive[0];
            break;

        case FlexemuOptionId::Drive1:
            optionsToWrite.drive[1] = previousOptions.drive[1];
            break;

        case FlexemuOptionId::Drive2:
            optionsToWrite.drive[2] = previousOptions.drive[2];
            break;

        case FlexemuOptionId::Drive3:
            optionsToWrite.drive[3] = previousOptions.drive[3];
            break;

        case FlexemuOptionId::CanFormatDrive0:
            optionsToWrite.canFormatDrive[0] =
                previousOptions.canFormatDrive[0];
            break;

        case FlexemuOptionId::CanFormatDrive1:
            optionsToWrite.canFormatDrive[1] =
                previousOptions.canFormatDrive[1];
            break;

        case FlexemuOptionId::CanFormatDrive2:
            optionsToWrite.canFormatDrive[2] =
                previousOptions.canFormatDrive[2];
            break;

        case FlexemuOptionId::CanFormatDrive3:
            optionsToWrite.canFormatDrive[3] =
                previousOptions.canFormatDrive[3];
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

        case FlexemuOptionId::IsTerminalIgnoreESC:
            optionsToWrite.isTerminalIgnoreESC =
                previousOptions.isTerminalIgnoreESC;
            break;

        case FlexemuOptionId::IsTerminalIgnoreNUL:
            optionsToWrite.isTerminalIgnoreNUL =
                previousOptions.isTerminalIgnoreNUL;
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
        }

        optionsToWrite.isDirectoryDiskActive =
            previousOptions.isDirectoryDiskActive;
        optionsToWrite.version = VERSION;
    }

    BRcFile rcFile(fileName.c_str());
    bool ok;
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXVERSION, VERSION);
    rcFile.SetValue(FLEXINVERSE, optionsToWrite.isInverse ? 1 : 0);
    rcFile.SetValue(FLEXDISPLAYSMOOTH, optionsToWrite.isSmooth ? 1 : 0);
    rcFile.SetValue(FLEXCOLOR, optionsToWrite.color.c_str());
    rcFile.SetValue(FLEXNCOLORS, optionsToWrite.nColors);
    rcFile.SetValue(FLEXSCREENFACTOR, optionsToWrite.pixelSize);
    rcFile.SetValue(FLEXMONITOR, optionsToWrite.hex_file.c_str());
    rcFile.SetValue(FLEXDISKDIR, optionsToWrite.disk_dir.c_str());
    rcFile.SetValue(FLEXDISK0, optionsToWrite.drive[0].c_str());
    rcFile.SetValue(FLEXDISK1, optionsToWrite.drive[1].c_str());
    rcFile.SetValue(FLEXDISK2, optionsToWrite.drive[2].c_str());
    rcFile.SetValue(FLEXDISK3, optionsToWrite.drive[3].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE0, optionsToWrite.mdcrDrives[0].c_str());
    rcFile.SetValue(FLEXMDCRDRIVE1, optionsToWrite.mdcrDrives[1].c_str());
    rcFile.SetValue(FLEXRAMEXTENSION, optionsToWrite.isRamExtension ? 1 : 0);
    rcFile.SetValue(FLEXHIMEM, optionsToWrite.isHiMem ? 1 : 0);
    rcFile.SetValue(FLEXFLEXIBLEMMU, optionsToWrite.isFlexibleMmu ? 1 : 0);
    rcFile.SetValue(FLEXEUROCOM2V5, optionsToWrite.isEurocom2V5 ? 1 : 0);
    rcFile.SetValue(FLEXUNDOCUMENTED, optionsToWrite.use_undocumented ? 1 : 0);
    rcFile.SetValue(FLEXRTC, optionsToWrite.useRtc ? 1 : 0);
    auto str = toString<decltype(options.frequency)>(options.frequency, ok);
    if (ok)
    {
        rcFile.SetValue(FLEXFREQUENCY, str.c_str());
    }
    rcFile.SetValue(FLEXFORMATDRIVE0, optionsToWrite.canFormatDrive[0] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE1, optionsToWrite.canFormatDrive[1] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE2, optionsToWrite.canFormatDrive[2] ? 1 : 0);
    rcFile.SetValue(FLEXFORMATDRIVE3, optionsToWrite.canFormatDrive[3] ? 1 : 0);
    rcFile.SetValue(FLEXFILETIMEACCESS,
                    static_cast<int>(optionsToWrite.fileTimeAccess));
    rcFile.SetValue(FLEXTERMINALIGNOREESC,
            optionsToWrite.isTerminalIgnoreESC ? 1 : 0);
    rcFile.SetValue(FLEXTERMINALIGNORENUL,
            optionsToWrite.isTerminalIgnoreNUL ? 1 : 0);
    rcFile.SetValue(FLEXDIRECTORYDISKTRACKS,
            optionsToWrite.directoryDiskTracks);
    rcFile.SetValue(FLEXDIRECTORYDISKSECTORS,
            optionsToWrite.directoryDiskSectors);
    rcFile.SetValue(FLEXISDIRECTORYDISKACTIVE,
            optionsToWrite.isDirectoryDiskActive ? 1 : 0);
    rcFile.SetValue(FLEXPRINTFONT, optionsToWrite.printFont.c_str());
    rcFile.SetValue(FLEXPRINTPAGEBREAKDETECTED,
            optionsToWrite.isPrintPageBreakDetected ? 1 : 0);
    rcFile.SetValue(FLEXPRINTORIENTATION,
            optionsToWrite.printOrientation.c_str());
    rcFile.SetValue(FLEXPRINTPAGESIZE, optionsToWrite.printPageSize.c_str());
    rcFile.SetValue(FLEXPRINTUNIT, optionsToWrite.printUnit.c_str());
    rcFile.SetValue(FLEXPRINTOUTPUTWINDOWGEOMETRY,
            optionsToWrite.printOutputWindowGeometry.c_str());
    rcFile.SetValue(FLEXPRINTPREVIEWDIALOGGEOMETRY,
            optionsToWrite.printPreviewDialogGeometry.c_str());

    for (const auto &iter : optionsToWrite.printConfigs)
    {
        const auto key = std::string(FLEXPRINTCONFIG) + iter.first;
        rcFile.SetValue(key.c_str(), iter.second.c_str());
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
    reg.GetValue(FLEXDISKDIR, options.disk_dir);
    reg.GetValue(FLEXDISK0, options.drive[0]);
    reg.GetValue(FLEXDISK1, options.drive[1]);
    reg.GetValue(FLEXDISK2, options.drive[2]);
    reg.GetValue(FLEXDISK3, options.drive[3]);
    reg.GetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0]);
    reg.GetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1]);
    reg.GetValue(FLEXMONITOR, options.hex_file);
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

        if (fromString<T>(string_result, value))
        {
            options.frequency = value;
        }
    }

    if (!reg.GetValue(FLEXFORMATDRIVE0, int_result))
    {
        options.canFormatDrive[0] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE1, int_result))
    {
        options.canFormatDrive[1] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE2, int_result))
    {
        options.canFormatDrive[2] = (int_result != 0);
    }

    if (!reg.GetValue(FLEXFORMATDRIVE3, int_result))
    {
        options.canFormatDrive[3] = (int_result != 0);
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
    const auto rcFileName = (getHomeDirectory() += PATHSEPARATORSTRING) +=
        FLEXEMURC;
    BRcFile rcFile(rcFileName.c_str());
    rcFile.GetValue(FLEXVERSION, options.version);
    rcFile.GetValue(FLEXDISKDIR, options.disk_dir);
    rcFile.GetValue(FLEXDISK0, options.drive[0]);
    rcFile.GetValue(FLEXDISK1, options.drive[1]);
    rcFile.GetValue(FLEXDISK2, options.drive[2]);
    rcFile.GetValue(FLEXDISK3, options.drive[3]);
    rcFile.GetValue(FLEXMDCRDRIVE0, options.mdcrDrives[0]);
    rcFile.GetValue(FLEXMDCRDRIVE1, options.mdcrDrives[1]);
    rcFile.GetValue(FLEXMONITOR, options.hex_file);
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
        if (fromString<T>(string_result, value))
        {
            options.frequency = value;
        }
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE0, int_result))
    {
        options.canFormatDrive[0] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE1, int_result))
    {
        options.canFormatDrive[1] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE2, int_result))
    {
        options.canFormatDrive[2] = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXFORMATDRIVE3, int_result))
    {
        options.canFormatDrive[3] = (int_result != 0);
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

    if (!rcFile.GetValue(FLEXTERMINALIGNOREESC, int_result))
    {
        options.isTerminalIgnoreESC = (int_result != 0);
    }

    if (!rcFile.GetValue(FLEXTERMINALIGNORENUL, int_result))
    {
        options.isTerminalIgnoreNUL = (int_result != 0);
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
} // GetOptions

