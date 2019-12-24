/*
    mdcrtool.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2018-2019  W. Schwotzer

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
#include "mdcrtape.h"
#include "mdcrfs.h"
#include "bmembuf.h"
#include "fileread.h"
#include "flexerr.h"
#include "bdir.h"
#include <ctype.h>
#include <limits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <vector>

void syntax()
{
    std::cout << "mdcrtool syntax:\n"
              << " Write/Append files to a MDCR file."
                 " (MDCR file is created if it does not exist):\n"
              << "   mdcrtool [-t][-u] -o <mdcr_file> <bin_file> [<bin_file>...]\n"
              << " Create a new empty MDCR file:\n"
              << "   mdcrtool -c <mdcr_file>\n"
              << " Extract all files from a MDCR file:\n"
              << "   mdcrtool [-d<directory>] -x <mdcr_file>\n"
              << " List contents of a MDCR file:\n"
              << "   mdcrtool -l <mdcr_file>\n"
              << "\n"
              << "   <bin_file>:  A file in FLEX binary format.\n"
              << "   <mdcr_file>: A file in Philips MDCR format.\n"
              << "   -t: Truncate. Overwrite existing files on MDCR file.\n"
              << "       Default: Append files at the end of MDCR file.\n"
              << "   -u: Convert file names to uppercase.\n"
              << "       Default: Keep file name as is.\n"
              << "   -d <directory>: Directory where to extract files.\n"
              << "       Default: Current working directory.\n";

}

int WriteAppendToMdcrFile(const std::vector<const char *> &ifiles,
                      const char *ofile,
                      bool isTruncate, bool toUppercase)
{
    MiniDcrTapePtr mdcr;
    BMemoryBuffer memory(65536);

    try
    {
        // 1. try to open existing file
        mdcr = MiniDcrTape::Open(ofile);
    }
    catch (FlexException &)
    {
        try
        {
            // 2. try to create a new file
            mdcr = MiniDcrTape::Create(ofile);
        }
        catch (FlexException &)
        {
            std::cerr << "*** Error creating or opening " << ofile <<
                         " for output\n";
            return 1;
        }
    }

    if (mdcr->IsWriteProtected())
    {
        std::cerr << "*** Error: file " << ofile << " is write protected\n";
        return 1;
    }

    for (const char *ifile : ifiles)
    {
        size_t startAddress = 0;

        memory.Reset();
        auto result = load_hexfile(ifile, memory, startAddress);
        if (result < 0)
        {
            std::cerr << "*** Error in \"" << ifile << "\":"
                      << std::endl << "    ";
            print_hexfile_error(std::cerr, result);
            std::cerr << " Ignored." << std::endl;
            continue; // ignore reading input file. Continue with next one.
        }

        MdcrFileSystem mdcrfs;
        MdcrWriteMode mode = isTruncate ?
                                MdcrWriteMode::Truncate :
                                MdcrWriteMode::Append;

        MdcrStatus status = mdcrfs.WriteFile(
                ifile,
                memory,
                *mdcr.get(),
                mode,
                toUppercase);

        if (status != MdcrStatus::Success)
        {
            using T = std::underlying_type<MdcrStatus>::type;

            if (status == MdcrStatus::DoubleName)
            {
                std::cerr << "*** Warning: "
                          << mdcrErrors[static_cast<T>(status)]
                          << ". Ignored.\n";
                continue; // Double name error is ignored.
            }
            else
            {
                std::cerr << "*** Error: "
                          << mdcrErrors[static_cast<T>(status)]
                          << ". Mdcr file: " << ofile << ".\n";
                return 1;
            }
        }
    }

    return 0;
}

int CreateMdcrFile(const char *ofile)
{
    MiniDcrTapePtr mdcr;

    try
    {
        mdcr = MiniDcrTape::Create(ofile);
    }
    catch (FlexException &ex)
    {
        std::cerr << "*** Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

int ExtractFromMdcrFile(const char *targetDir, const char *ifile)
{
    MiniDcrTapePtr mdcr;

    try
    {
        mdcr = MiniDcrTape::Open(ifile);
    }
    catch (FlexException &ex)
    {
        std::cerr << "*** Error: " << ex.what() << "\n";
        return 1;
    }

    std::string sTargetDir;
    if (targetDir != nullptr)
    {
        sTargetDir = targetDir;
        if (!BDirectory::Exists(sTargetDir))
        {
            std::cerr << "*** Error: '" << targetDir << "' does not exist or is"
            " no directory.\n";
            return 1;
        }

        if (sTargetDir.at(sTargetDir.size() - 1) != PATHSEPARATOR)
        {
            sTargetDir += PATHSEPARATORSTRING;
        }
    }

    MdcrFileSystem mdcrfs;

    auto status = mdcrfs.ForEachFile(*mdcr.get(),
                       [&sTargetDir](const std::string &filename,
                          BMemoryBuffer &memory)
    {
        auto pos = filename.find_last_not_of(' ');
        std::string outFilename(filename.c_str(), pos+1);
        outFilename = sTargetDir + outFilename;

        auto result = write_flex_binary(outFilename.c_str(), memory);
        if (result < 0)
        {
            std::cerr << "*** Error in \"" << outFilename << "\":"
                      << std::endl << "    ";
            print_hexfile_error(std::cerr, result);
            std::cerr << " Ignored." << std::endl;
        }

        return MdcrStatus::Success;
    });

    if (status != MdcrStatus::Success)
    {
        using T = std::underlying_type<MdcrStatus>::type;

        std::cerr << "*** Error: "
                  << mdcrErrors[static_cast<T>(status)]
                  << ". Mdcr file: " << ifile << ".\n";
        return 1;
    }

    return 0;
}

int ListContentOfMdcrFile(const char *ifile)
{
    MiniDcrTapePtr mdcr;

    try
    {
        mdcr = MiniDcrTape::Open(ifile);
    }
    catch (FlexException &ex)
    {
        std::cerr << "*** Error: " << ex.what() << "\n";
        return 1;
    }

    MdcrFileSystem mdcrfs;

    auto status = mdcrfs.ForEachFile(*mdcr.get(),
                       [](const std::string &filename,
                          BMemoryBuffer &memory)
    {
        if (memory.GetAddressRanges().size() != 1)
        {
            std::cout << filename << " unknown address range\n";
        }
        else
        {
            const auto addrRange = memory.GetAddressRanges()[0];
            std::cout
                << filename << std::hex << std::uppercase
                << " " << std::setw(4) << std::setfill('0') << addrRange.lower()
                << " " << std::setw(4) << std::setfill('0') << addrRange.upper()
                << "\n";
        }
        return MdcrStatus::Success;
    });

    if (status != MdcrStatus::Success)
    {
        using T = std::underlying_type<MdcrStatus>::type;

        std::cerr << "*** Error: "
                  << mdcrErrors[static_cast<T>(status)]
                  << ". Mdcr file: " << ifile << ".\n";
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::string optstr("o:c:x:l:tuhd:");
    std::vector<const char *>ifiles;
    const char *ofile = nullptr;
    const char *ifile = nullptr;
    const char *targetDir = nullptr;
    bool isTruncate = false;
    bool isUppercase = false;
    int command = 0;
    int result;
    int index;

    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'o': ofile = optarg;
                      command = result;
                      break;
            case 'c': ofile = optarg;
                      command = result;
                      break;
            case 'x': ifile = optarg;
                      command = result;
                      break;
            case 'l': ifile = optarg;
                      command = result;
                      break;
            case 'd': targetDir = optarg;
                      break;
            case 't': isTruncate = true;
                      break;
            case 'u': isUppercase = true;
                      break;
            case 'h': syntax();
                      return 0;
            case '?':
                      if (optopt != 'o' && !isprint(optopt))
                      {
                          std::cerr << "Unknown option character '\\x" <<
                                       std::hex << optopt << "'.\n";
                      }
                      return 1;
            default:  return 1;
        }
    }

    for (index = optind; index < argc; index++)
    {
        ifiles.push_back(argv[index]);
    }

    if (command == 0 ||
        (command == 'o' && (ifiles.empty() || ofile == nullptr)) ||
        (command == 'c' && (!ifiles.empty() || ofile == nullptr ||
                            isTruncate || isUppercase)) ||
        (command == 'x' && (!ifiles.empty() || ifile == nullptr ||
                            isTruncate || isUppercase)) ||
        (command == 'l' && (!ifiles.empty() || ifile == nullptr ||
                            isTruncate || isUppercase)) ||
        (command != 'x' && targetDir != nullptr))
    {
        syntax();
        return 1;
    }

    switch(command)
    {
        case 'o':
            return WriteAppendToMdcrFile(ifiles, ofile, isTruncate, isUppercase);
            break;

        case 'c':
            return CreateMdcrFile(ofile);
            break;

        case 'x':
            return ExtractFromMdcrFile(targetDir, ifile);
            break;

        case 'l':
            return ListContentOfMdcrFile(ifile);
            break;

        default:
            break;
    }

    return 0;
}

