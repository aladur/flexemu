/*
    flex2hex.cpp


    flex2hex, a utility to convert FLEX binary files to Intel Hex or
    Motorola S-Record files.
    Copyright (C) 2019  W. Schwotzer

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
#include "bmembuf.h"
#include "fileread.h"
#include "flexerr.h"
#include <ctype.h>
#include <limits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
    #include <sys/types.h>
    #include <unistd.h>
#endif


enum class FileType
{
    Unknown,
    IntelHex,
    MotorolaSRec,
};

void syntax()
{
    std::cout <<
        "flex2hex syntax:\n"
        " Convert FLEX binary file(s) to Intel Hex or Motorola S-Record File:\n"
        "   flex2hex [-i|-m][-y][-v] -o <hex_file> <flex_bin_file>\n"
        "   flex2hex [-i|-m][-y][-v] <flex_bin_file> [<flex_bin_file>...]\n"
        "   flex2hex -h\n\n"
        "   <flex_bin_file>: A input file in FLEX binary format.\n"
        "   -o <hex_file>:   A output file.\n"
        "   -i:              hex_file has Intel Hex format (*.hex).\n"
        "   -m:              hex_file has Motorola S-Record format (*.s19).\n"
        "   -y:              Overwrite existing file(s) without confirmation.\n"
        "   -v:              Verbose output.\n"
        "   -h:              Print this help and exit.\n";
}

int ConvertFlexToHex(const char *ifile, const char *ofile,
                     FileType ofiletype, int verbose)
{
    BMemoryBuffer memory(65536);
    size_t startAddress = std::numeric_limits<size_t>::max();

    // TODO only support FLEX binary
    auto result = load_hexfile(ifile, memory, startAddress);
    if (result < 0)
    {
        std::cerr << "*** Error in \"" << ifile << "\":" << std::endl << "    ";
        print_hexfile_error(std::cerr, result);
        std::cerr << std::endl;
        return 1;
    }

    switch(ofiletype)
    {
        case FileType::IntelHex:
                  result = write_intelhex(ofile, memory, startAddress);
                  break;

        case FileType::MotorolaSRec:
                  result = write_motorola_srec(ofile, memory, startAddress);
                  break;

        case FileType::Unknown:
                  std::cerr << "*** No file format specified" << std::endl;
                  return 1;
    }

    if (result < 0)
    {
        std::cerr << "*** Error in \"" << ofile << "\":" << std::endl << "    ";
        print_hexfile_error(std::cerr, result);
        std::cerr << std::endl;
        return 1;
    }

    if (verbose > 0)
    {
        std::cout << " " << ifile << " converted to " << ofile << ".\n";
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::map<int, FileType> fileTypes{
        { 'm', FileType::MotorolaSRec },
        { 'i', FileType::IntelHex },
    };
    std::string optstr("himo:vy");
    std::vector<std::string> ifiles;
    std::string ofilePrefered;
    FileType ofiletype = FileType::Unknown;
    bool isOverwriteAlways = false;
    int verbose = 0;
    int result;

    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'o': ofilePrefered = optarg;
                      break;

            case 'i': 
            case 'm': 
                      if (ofiletype != FileType::Unknown &&
                          ofiletype != fileTypes[result])
                      {
                          std::cerr << "*** Error: Only one of -m or -i can "
                                       "be used at a time." << std::endl;
                          syntax();
                          return 1;
                      }
                      ofiletype = fileTypes[result];
                      break;

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
        ifiles.emplace_back(argv[index]);
        ++index;
    }

    if (ofiletype == FileType::Unknown)
    {
        ofiletype = FileType::MotorolaSRec;
    }

    if (ifiles.empty())
    {
        std::cerr << "*** Error: No FLEX binary file specified" << std::endl;
        syntax();
        return 1;
    }

    if ((ifiles.size() > 1 && !ofilePrefered.empty()))
    {
        std::cerr << "*** Error: Option -o only supported for one "
                     "FLEX binary file" << std::endl;
        syntax();
        return 1;
    }

    result = 0;

    for (const auto &ifile : ifiles)
    {
        struct stat status;
        auto ofile(ofilePrefered);

        if (ofile.empty())
        {
            ofile = getFileStem(ifile);
            switch (ofiletype)
            {
                case FileType::IntelHex:
                    ofile += ".hex";
                    break;

                case FileType::MotorolaSRec:
                    ofile += ".s19";
                    break;

                case FileType::Unknown:
                    std::cerr << "*** No file format specified. "
                                 "Conversion skipped" << std::endl;
                    continue;
            }
        }

        if (!stat(ofile.c_str(), &status))
        {
            if (!S_ISREG(status.st_mode))
            {
                std::cerr << "*** File " << ofile
                          << " exists but is no file. Conversion skipped."
                          << std::endl;
                continue;
            }

            if (!isOverwriteAlways)
            {
                std::string input;

                // User confirmation to overwrite file.
                while (input.empty() || 
                       (tolower(input[0]) != 'y' && tolower(input[0]) != 'n'))
                {
                    std::cout << "File " << ofile
                              << " already exists. Overwrite [Y,n]: ";
                    std::getline(std::cin, input);
                    input = ltrim(input);
                }

                if (!input.empty() && tolower(input[0]) == 'n')
                {
                    if (verbose > 0)
                    {
                        std::cout << " Conversion to " << ofile << " skipped."
                                  << std::endl;
                    }

                    continue;
                }
            }
        }

        result |= ConvertFlexToHex(ifile.c_str(), ofile.c_str(),
                                   ofiletype, verbose);
    }

    return result;
}

