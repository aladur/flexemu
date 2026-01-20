/*
    hex2flex.cpp


    hex2flex, a utility to convert Intel Hex or Motorola S-Record files
    to FLEX binary files.
    Copyright (C) 2026  W. Schwotzer

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


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "config.h"
#include "typedefs.h"
#include "misc1.h"
#include "free.h"
#include "bmembuf.h"
#include "fileread.h"
#include "wmain.h"
#ifdef _WIN32
#include "wingtopt.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cctype>
#include <cstdint>
#include <limits>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


enum class FileType : uint8_t
{
    Unknown,
    RawBinary,
    FlexBinary,
};

static void version()
{
    flx::print_versions(std::cout, "hex2flex");
}

static void syntax()
{
    std::cout <<
        "hex2flex syntax:\n"
        " Convert Intel Hex or Motorola S-Record File to FLEX binary file(s)\n"
        "   hex2flex [-f|-b][-y][-v] -o <flex_bin_file> <hex_file>\n"
        "   hex2flex [-f|-b][-y][-v] <hex_file> [<hex_file>...]\n"
        "   hex2flex -h\n\n"
        "   <flex_bin_file>: A input file in Hex format.\n"
        "   -o <hex_file>:   A output file.\n"
        "   -b:              flex_bin_file has raw binary format (*.bin).\n"
        "                    Address range gaps are filled up with 0.\n"
        "   -f:              flex_bin_file has FLEX binary format (*.cmd), "
        "the default.\n"
        "   -y:              Overwrite existing file(s) without confirmation.\n"
        "   -v:              Verbose output.\n"
        "   -V:              Print version number and exit.\n"
        "   -h:              Print this help and exit.\n";
}

static int ConvertHexToFlex(const fs::path &ifile, const fs::path &ofile,
        FileType ofiletype, int verbose)
{
    BMemoryBuffer memory(65536);
    DWord startAddress = std::numeric_limits<DWord>::max();

    auto result = load_hexfile(ifile, memory, startAddress, true);
    if (result < 0)
    {
        std::cerr << "*** Error in " << ifile << ":\n    ";
        print_hexfile_error(std::cerr, result);
        std::cerr << '\n';
        return 1;
    }

    switch(ofiletype)
    {
        case FileType::FlexBinary:
                  result = write_flex_binary(ofile, memory, startAddress);
                  break;

        case FileType::RawBinary:
                  result = write_raw_binary(ofile, memory, startAddress);
                  break;

        case FileType::Unknown:
                  std::cerr << "*** No file format specified\n";
                  return 1;
    }

    if (result < 0)
    {
        std::cerr << "*** Error in " << ofile << ":\n    ";
        print_hexfile_error(std::cerr, result);
        std::cerr << '\n';
        return 1;
    }

    if (verbose > 0)
    {
        std::cout << " " << ifile << " converted to " << ofile << ".\n";
    }

    return 0;
}

// Compatiblitity to main function parameters.
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
int flx::main(int argc, char *argv[])
{
    static const std::map<int, FileType> fileTypes{
        { 'f', FileType::FlexBinary },
        { 'b', FileType::RawBinary },
    };
    std::string optstr("hfbo:vVy");
    std::vector<fs::path> ifiles;
    fs::path ofilePrefered;
    FileType ofiletype = FileType::Unknown;
    bool isOverwriteAlways = false;
    int verbose = 0;
    int result;

#ifdef _WIN32
    // Set console input and output code page to UTF-8. This makes the
    // remaining code portable between Unix like OS and Windows.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    opterr = 1;
    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'o': ofilePrefered = fs::u8path(optarg);
                      break;

            case 'f':
            case 'b':
                      if (ofiletype != FileType::Unknown &&
                          ofiletype != fileTypes.at(result))
                      {
                          std::cerr << "*** Error: Only one of -f or -b can "
                                       "be used at a time.\n";
                          syntax();
                          return 1;
                      }
                      ofiletype = fileTypes.at(result);
                      break;

            case 'V': version();
                      return 0;

            case 'h': syntax();
                      return 0;

            case 'v': verbose = 1;
                      break;

            case 'y': isOverwriteAlways = true;
                      break;

            case '?':
                      if (optopt != 'o' && !isprint(optopt))
                      {
                          std::cerr << "*** Unknown option character '\\x" <<
                                       std::hex << optopt << "'.\n";
                      }
                      return 1;

            default:  return 1;
        }
    }

    auto index = optind;
    while (index < argc)
    {
        ifiles.emplace_back(fs::u8path(argv[index]));
        ++index;
    }

    if (ofiletype == FileType::Unknown)
    {
        ofiletype = FileType::FlexBinary;
    }

    if (ifiles.empty())
    {
        std::cerr << "*** Error: No input file specified\n";
        syntax();
        return 1;
    }

    if ((ifiles.size() > 1 && !ofilePrefered.empty()))
    {
        std::cerr << "*** Error: Option -o only supported for one "
                     "input file\n";
        syntax();
        return 1;
    }

    result = 0;

    for (const auto &ifile : ifiles)
    {
        auto ofile(ofilePrefered);

        if (ofile.empty())
        {
            ofile = ifile.stem();
            switch (ofiletype)
            {
                case FileType::FlexBinary:
                    ofile += u8".cmd";
                    break;

                case FileType::RawBinary:
                    ofile += u8".bin";
                    break;

                case FileType::Unknown:
                    std::cerr << "*** No file format specified. "
                                 "Conversion skipped\n";
                    continue;
            }
        }

        const auto status = fs::status(ofile);
        if (fs::exists(status))
        {
            if (!fs::is_regular_file(status))
            {
                std::cerr << "*** File " << ofile
                          << " exists but is no file. Conversion skipped.\n";
                continue;
            }

            if (!isOverwriteAlways)
            {
                std::string input;

                // User confirmation to overwrite file.
                while (input.empty() ||
                       (::tolower(input[0]) != 'y' && ::tolower(input[0]) != 'n'))
                {
                    std::cout << "File " << ofile
                              << " already exists. Overwrite [Y,n]: ";
                    std::getline(std::cin, input);
                    input = flx::ltrim(std::move(input));
                }

                if (!input.empty() && ::tolower(input[0]) == 'n')
                {
                    if (verbose > 0)
                    {
                        std::cout << " Conversion to " << ofile << " skipped."
                                  << '\n';
                    }

                    continue;
                }
            }
        }

        result = ConvertHexToFlex(ifile, ofile, ofiletype, verbose);
    }

    return result;
}

