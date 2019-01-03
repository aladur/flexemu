/*
    dcrtool.cpp


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
#include <ctype.h>
#include <limits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <vector>

void syntax()
{
    std::cout << "dcrtool syntax:\n"
              << " Write/Append files to a DCR file."
                 " (DCR file is created if it does not exist):\n"
              << "   dcrtool [-t][-u] -o <dcr_file> <bin_file> [<bin_file>...]\n"
              << " Create a new empty DCR file:\n"
              << "   dcrtool -c <dcr_file>\n"
              << " Extract all files from a DCR file:\n"
              << "   dcrtool -x <dcr_file>\n"
              << " List contents of a DCR file:\n"
              << "   dcrtool -l <dcr_file>\n"
              << "\n"
              << "   <bin_file>: A file in FLEX binary format.\n"
              << "   <dcr_file>: A file in Philips MDCR format.\n"
              << "   -t: Truncate. Overwrite existing files on DCR file.\n"
              << "       Default: Append files at the end of DCR file.\n"
              << "   -u: Convert file names to uppercase.\n"
              << "       Default: Keep file name as is.\n";

}

int WriteAppendToDcrFile(const std::vector<const char *> &ifiles,
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
        memory.ResetStartEndAddress();
        auto result = load_hexfile(ifile, memory);
        if (result == -1)
        {
            std::cerr << "*** Error opening or reading from " << ifile <<
                         ". Ignored.\n";
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

int CreateDcrFile(const char *ofile)
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

int ExtractFromDcrFile(const char *ifile)
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
    std::string filename;
    BMemoryBuffer memory(65536);

    auto status = mdcrfs.ForEachFile(*mdcr.get(),
                       [](const std::string &filename,
                          BMemoryBuffer &memory)
    {
        auto pos = filename.find_last_not_of(' ');
        std::string outFilename(filename.c_str(), pos+1);

        auto result = write_flex_binary(outFilename.c_str(), memory);
        if (result < 0)
        {
            std::cerr << "*** Error opening or writing to " << outFilename <<
                         ". Ignored.\n";
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

int ListContentOfDcrFile(const char *ifile)
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
    std::string filename;
    BMemoryBuffer memory(65536);

    auto status = mdcrfs.ForEachFile(*mdcr.get(),
                       [](const std::string &filename,
                          BMemoryBuffer &memory)
    {
        auto addrRange = memory.GetStartEndAddress();
        std::cout << filename << std::hex << std::uppercase
                  << " " << std::setw(4) << std::setfill('0') << addrRange.first
                  << " " << std::setw(4) << std::setfill('0') << addrRange.second << "\n";
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
    std::string optstr("o:c:x:l:tuh");
    std::vector<const char *>ifiles;
    const char *ofile = nullptr;
    const char *ifile = nullptr;
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
                            isTruncate || isUppercase)))
    {
        syntax();
        return 1;
    }

    switch(command)
    {
        case 'o':
            return WriteAppendToDcrFile(ifiles, ofile, isTruncate, isUppercase);
            break;

        case 'c':
            return CreateDcrFile(ofile);
            break;

        case 'x':
            return ExtractFromDcrFile(ifile);
            break;

        case 'l':
            return ListContentOfDcrFile(ifile);
            break;

        default:
            break;
    }

    return 0;
}

