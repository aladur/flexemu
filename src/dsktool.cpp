/*
    dsktool.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include <regex>
#include "bdir.h"
#include "benv.h"
#include "flexerr.h"
#include "efiletim.h"
#include "fdirent.h"
#include "filecntb.h"
#include "rfilecnt.h"
#include "dircont.h"
#include "ifilecnt.h"
#include "fcopyman.h"
#include "fcinfo.h"
#include "filfschk.h"
#include "ffilebuf.h"
#include "filecnts.h"
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


static std::vector<std::string> GetMatchingFilenames(FlexDisk &container,
        const std::vector<std::regex> &regexs)
{
    FlexDiskIterator iter;
    std::vector<std::string> allFilenames;

    for (iter = container.begin(); iter != container.end(); ++iter)
    {
        allFilenames.emplace_back((*iter).GetTotalFileName());
    }

    if (regexs.empty())
    {
        return allFilenames;
    }

    std::vector<std::string> result;
    std::set<size_t> fileIndices; // To make entries in result unique

    for (const auto &regex : regexs)
    {
        size_t index = 0U;
        for (const std::string &filename : allFilenames)
        {
            if (std::regex_search(filename, regex) &&
                fileIndices.find(index) == fileIndices.end())
            {
                result.emplace_back(filename);
                fileIndices.emplace(index);
            }
            ++index;
        }
    }

    return result;
}

static int FormatFlexDiskFile(const std::string &dsk_file, DiskType disk_type,
        int tracks, int sectors, char default_answer, bool verbose,
        const char *bsFile)
{
    struct stat sbuf{};

    if (!stat(dsk_file.c_str(), &sbuf))
    {
        if (!S_ISREG(sbuf.st_mode))
        {
            std::cerr << "*** Error: " << dsk_file <<
                         " exists but is no regular file. Aborted.\n";
            return 1;
        }

        std::string question(dsk_file);

        question += " already exists. Overwrite?";
        if (flx::askForInput(question, "yn", default_answer))
        {
            unlink(dsk_file.c_str());
        }
        else
        {
            if (default_answer != '?')
            {
                std::cout << dsk_file << " already exists. Skipped.\n";
            }
            return 0;
        }
    }

    try
    {
        std::unique_ptr<FlexDisk> container;
        auto fileTimeAccess = FileTimeAccess::NONE;

        container.reset(FlexDisk::Create(
                        dsk_file,
                        fileTimeAccess,
                        tracks, sectors,
                        disk_type,
                        bsFile));

        if (container && verbose)
        {
            std::cout << "Successfully created " <<
                flx::getFileName(dsk_file) << " with " <<
                tracks << "-" << sectors << " tracks-sectors.\n";
        }
    }
    catch (FlexException &ex)
    {
        std::cerr << "*** " << ex.what() << ".\n";

        return 1;
    }

    return 0;
}

static int ExtractDskFile(const std::string &target_dir, bool verbose,
        bool convert_text, char default_answer, const std::string &dsk_file,
        const std::vector<std::regex> &regexs, FileTimeAccess fileTimeAccess)
{
    if (verbose)
    {
        std::cout << "Extracting from '" << dsk_file << "' into '" <<
                     target_dir << "' ... \n";
    }

    FlexCopyManager::autoTextConversion = convert_text;
    const auto mode = std::ios::in | std::ios::binary;
    FlexRamDisk src{dsk_file, mode, fileTimeAccess};
    FlexDirectoryDiskByFile dest{target_dir, fileTimeAccess};
    size_t count = 0;
    size_t random_count = 0;
    size_t byte_size = 0;
    size_t errors = 0;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }

    auto matchedFilenames = GetMatchingFilenames(src, regexs);

    for (const auto &filename : matchedFilenames)
    {
        std::string result = "ok";
        std::string what;
        bool isText = false;

        try
        {
            FlexDirEntry dirEntry;

            if (dest.FindFile(filename, dirEntry))
            {
                auto question = filename + " already exists. Overwrite?";
                if (flx::askForInput(question, "yn", default_answer))
                {
                    dest.DeleteFile(dirEntry.GetTotalFileName());
                }
                else
                {
                    if (default_answer != '?')
                    {
                        std::cout << filename << " already exists. Skipped.\n";
                    }
                    continue;
                }
            }

            isText = src.FileCopy(filename, filename, dest);

            ++count;
            if (src.FindFile(filename, dirEntry))
            {
                byte_size += dirEntry.GetFileSize();
                random_count += dirEntry.IsRandom() ? 1 : 0;
            }
        }
        catch (FlexException &ex)
        {
            result = "failed";
            what = ex.what();
            ++errors;
        }

        if (verbose)
        {
            std::string fileType;

            if (what.empty())
            {
                fileType = isText ? "text file" : "binary file";
            }

            std::cout << " extracting " << fileType << " " << filename <<
                         " ... " << result << '\n';
        }
        if (!what.empty())
        {
            std::cerr << "   *** Error: " << what << '\n';
        }
    }

    if (verbose)
    {
        std::cout << " " << count << " file(s), " << random_count <<
                     " random file(s), ";
        if (errors != 0)
        {
            std::cout << errors << " errors, ";
        }
        auto kbyte_size = byte_size / 1024;
        std::cout << "total size: " << kbyte_size << " KByte.\n";
    }

    return 0;
}

static int ExtractDskFiles(std::string target_dir, bool verbose,
        bool convert_text, char default_answer,
        const std::vector<std::string> &dsk_files,
        const std::vector<std::regex> &regexs, FileTimeAccess fileTimeAccess)
{
    if (target_dir.empty())
    {
        target_dir = ".";
    }

    if (!BDirectory::Exists(target_dir))
    {
        std::cerr <<
            "*** Error: '" << target_dir << "' does not exist or is"
            " no directory.\n" <<
            "    Extraction aborted.\n";
        return 1;
    }

    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            int result = ExtractDskFile(target_dir, verbose, convert_text,
                                        default_answer, dsk_file, regexs,
                                        fileTimeAccess);
            if (result != 0)
            {
                // Abort after fatal error.
                return result;
            }
        }
        catch (FlexException &ex)
        {
            if (verbose)
            {
                std::cout << " failed.\n";
            }
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Extraction from '" << dsk_file << "' aborted.\n";
        }
    }

    return 0;
}

static int ListDirectoryOfDskFile(const std::string &dsk_file,
        const std::vector<std::regex> &regexs, FileTimeAccess fileTimeAccess)
{
    const auto mode = std::ios::in | std::ios::binary;
    FlexRamDisk src{dsk_file, mode, fileTimeAccess};
    FlexDiskIterator iter;
    FlexDiskAttributes diskAttributes;
    unsigned int number = 0;
    bool hasAttributes = false;
    int sumSectors = 0;
    int largest = 0;
    const auto format = BDate::Format::D2MSU3Y4;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }

    if (src.GetDiskAttributes(diskAttributes))
    {
        hasAttributes = true;
        std::cout <<
            "FILE: " << flx::getFileName(dsk_file) << "  " <<
            "DISK: " << diskAttributes.GetName() <<
            " #" << diskAttributes.GetNumber() <<
            "  CREATED: " << diskAttributes.GetDate().GetDateString(format) <<
            "\n";
    }
    else
    {
        std::cerr << "Error reading disk image attributes from " <<
                     flx::getFileName(dsk_file) << "\n";
    }

    std::cout << "FILE#   NAME   TYPE  BEGIN   END   SIZE    DATE      ";
    if ((fileTimeAccess & FileTimeAccess::Get) == FileTimeAccess::Get)
    {
        std::cout << " TIME ";
    }
    std::cout << " PRT   RND\n\n";

    auto matchedFilenames = GetMatchingFilenames(src, regexs);

    for (const auto &filename : matchedFilenames)
    {
        FlexDirEntry dirEntry;
        int startTrack;
        int startSector;
        int endTrack;
        int endSector;

        if (!src.FindFile(filename, dirEntry))
        {
            continue;
        }
        ++number;
        dirEntry.GetStartTrkSec(startTrack, startSector);
        dirEntry.GetEndTrkSec(endTrack, endSector);

        int sectors = static_cast<int>(dirEntry.GetFileSize() / SECTOR_SIZE);
        sumSectors += sectors;
        largest = std::max(sectors, largest);

        std::cout << fmt::format(
                  "{:5}  {:<8}.{:<3}  {:02X}-{:02X}  {:02X}-{:02X} {:5}  {:11} ",
                    number, dirEntry.GetFileName(),
                    dirEntry.GetFileExt(), startTrack, startSector,
                    endTrack, endSector, sectors,
                    dirEntry.GetDate().GetDateString(format));
        if ((fileTimeAccess & FileTimeAccess::Get) == FileTimeAccess::Get)
        {
            std::cout <<
                dirEntry.GetTime().AsString(BTime::Format::HHMM) << " ";
        }
        std::cout << fmt::format("{:<4} {}\n", dirEntry.GetAttributesString(),
                     (dirEntry.IsRandom() ? "R" : ""));
    }

    if (hasAttributes)
    {
        std::cout << "\n    " <<
                     "FILES=" << number <<
                     ", SECTORS=" << sumSectors <<
                     ", LARGEST=" << largest <<
                     ", FREE=" << (diskAttributes.GetFree() / SECTOR_SIZE) <<
                     "\n\n";
    }

    return 0;
}

static int ListDirectoryOfDskFiles(const std::vector<std::string> &dsk_files,
        const std::vector<std::regex> &regexs, FileTimeAccess fileTimeAccess)
{
    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            ListDirectoryOfDskFile(dsk_file, regexs, fileTimeAccess);
        }
        catch (FlexException &ex)
        {
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       List directory of '" << dsk_file << "' aborted.\n";
        }
    }

    return 0;
}

static int SummaryOfDskFile(const std::string &dsk_file,
        uint64_t &sum_files, uint64_t&sum_size, uint64_t&sum_free,
        bool verbose)
{
    auto fileTimeAccess = FileTimeAccess::NONE;
    const auto mode = std::ios::in | std::ios::binary;
    FlexRamDisk src{dsk_file, mode, fileTimeAccess};
    FlexDiskIterator iter;
    FlexDiskAttributes diskAttributes;
    const auto format = BDate::Format::D2MSU3Y4;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }

    if (src.GetDiskAttributes(diskAttributes))
    {
        uint64_t file_count = 0;
        int tracks;
        int sectors;

        diskAttributes.GetTrackSector(tracks, sectors);

        for (iter = src.begin(); iter != src.end(); ++iter)
        {
            ++file_count;
        }
        sum_files += file_count;

        auto name = diskAttributes.GetName();
        if (name.empty())
        {
            name = "\"\"";
        }

        std::string file = verbose ? dsk_file : flx::getFileName(dsk_file);

        std::cout << fmt::format(
            "{} {:<12} {:<5} {:<2}-{:<2} {:<5} {:<5} {:<5} {}\n",
            diskAttributes.GetDate().GetDateString(format), name,
            diskAttributes.GetNumber(),
            tracks, sectors, file_count,
            diskAttributes.GetTotalSize() / SECTOR_SIZE,
            diskAttributes.GetFree() / SECTOR_SIZE, file);
        sum_size += (diskAttributes.GetTotalSize() / SECTOR_SIZE);
        sum_free += (diskAttributes.GetFree() / SECTOR_SIZE);
    }
    else
    {
        std::cerr << "Error reading disk image attributes for " <<
                     flx::getFileName(dsk_file) << "\n";
    }

    return 0;
}

static int SummaryOfDskFiles(const std::vector<std::string> &dsk_files,
        bool verbose)
{
    uint64_t sum_files = 0U;
    uint64_t sum_size = 0U;
    uint64_t sum_free = 0U;

    std::cout <<
        "DATE        DISKNAME     #     TT-SS FILES SIZE  FREE  FILE\n" <<
        "                                           [SECTORS]\n";

    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            SummaryOfDskFile(dsk_file, sum_files, sum_size, sum_free, verbose);
        }
        catch (FlexException &ex)
        {
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Summary of '" << dsk_file << "' aborted.\n";
        }
    }

    if (dsk_files.size() > 1)
    {
        std::cout <<
            "FILES: " << sum_files <<
            " SIZE: " << sum_size << " Sectors/" <<
            (sum_size / 4) << " KByte" <<
            " FREE: " << sum_free << " Sectors/" <<
            (sum_free / 4) << " KByte\n";
    }

    return 0;
}

static int InjectToDskFile(const std::string &dsk_file, bool verbose,
        const std::vector<std::string> &files, char default_answer,
        bool isConvertText, FileTimeAccess fileTimeAccess)
{
    FlexCopyManager::autoTextConversion = isConvertText;
    const auto mode = std::ios::in | std::ios::out | std::ios::binary;
    FlexRamDisk dst{dsk_file, mode, fileTimeAccess};

    if (!dst.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, dst.GetPath());
    }

    for (const auto &file : files)
    {
        bool isSuccess = true;
        bool isText = false;
        FlexFileBuffer fileBuffer;

        if (!fileBuffer.ReadFromFile(file, fileTimeAccess, true))
        {
            std::cerr <<
                "   *** Error: Reading from " << file << ". Aborted.\n";
            continue;
        }

        if (isConvertText && fileBuffer.IsTextFile())
        {
            fileBuffer.ConvertToFlexTextFile();
            isText = true;
        }

        try
        {
            FlexDirEntry dirEntry;

            if (dst.FindFile(fileBuffer.GetFilename(), dirEntry))
            {
                std::string question(fileBuffer.GetFilename());

                question += " already exists. Overwrite?";
                if (flx::askForInput(question, "yn", default_answer))
                {
                    dst.DeleteFile(dirEntry.GetTotalFileName());
                }
                else
                {
                    if (default_answer != '?')
                    {
                        std::cout << fileBuffer.GetFilename() <<
                            " already exists. Skipped.\n";
                    }
                    continue;
                }
            }

            dst.WriteFromBuffer(fileBuffer);
        }
        catch (FlexException &ex)
        {
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Injecting of " << file << " aborted.\n";
            isSuccess = false;
        }

        if (isSuccess && verbose)
        {
            std::string fileType = isText ? "text file" : "binary file";

            std::cout << "Injecting " << fileType << " " << file << " ... Ok\n";
        }
    }

    return 0;
}

static int DeleteFromDskFile(const std::string &dsk_file, bool verbose,
        const std::vector<std::regex> &regexs, char default_answer)
{
    auto fileTimeAccess = FileTimeAccess::NONE;
    const auto mode = std::ios::in | std::ios::out | std::ios::binary;
    FlexRamDisk src{dsk_file, mode, fileTimeAccess};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }

    if (regexs.empty())
    {
        return 0;
    }

    auto matchedFilenames = GetMatchingFilenames(src, regexs);

    for (auto &flex_file : matchedFilenames)
    {
        bool isSuccess = true;

        try
        {
            FlexDirEntry dirEntry;

            if (src.FindFile(flex_file, dirEntry))
            {
                std::stringstream question;

                question << "Delete " << flex_file << "?";
                if (flx::askForInput(question.str(), "yn", default_answer))
                {
                    src.DeleteFile(flex_file);
                }
                else
                {
                    if (verbose)
                    {
                        std::cout << "Deleting " << flex_file << " skipped.\n";
                    }
                    continue;
                }
            }
            else
            {
                std::cerr << flex_file << " not found, ignored.\n";
                isSuccess = false;
            }
        }
        catch (FlexException &ex)
        {
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Deleting of " << flex_file << " aborted.\n";
            isSuccess = false;
        }

        if (isSuccess && verbose)
        {
            std::cout << "Deleting " << flex_file << " ... Ok\n";
        }
    }

    return 0;
}

static int CheckConsistencyOfDskFile(const std::string &dsk_file,
        bool verbose, bool debug_output, FileTimeAccess fileTimeAccess)
{
    const auto mode = std::ios::in | std::ios::binary;
    FlexRamDisk src{dsk_file, mode, fileTimeAccess};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }

    FlexDiskCheck check(src, fileTimeAccess);

    std::cout << "Check " << dsk_file << " ...";
    if (check.CheckFileSystem())
    {
        std::cout << " Ok\n";
    }
    else
    {
        std::cout << " " << check.GetStatisticsString() << "\n";

        if (verbose)
        {
            for (const auto &result : check.GetResult())
            {
                std::cout << "  " << result << '\n';
            }
        }
    }

    if (debug_output)
    {
        check.DebugDump(std::cerr);
    }

    return 0;
}

static int CheckConsistencyOfDskFiles(const std::vector<std::string> &dsk_files,
        bool verbose, bool debug_output, FileTimeAccess fileTimeAccess)
{
    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            CheckConsistencyOfDskFile(dsk_file, verbose, debug_output,
                    fileTimeAccess);
        }
        catch (FlexException &ex)
        {
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Check consistency of '" << dsk_file << "' aborted.\n";
        }
    }

    return 0;
}

static int CopyFromToDskFile(const std::string &src_dsk_file,
        const std::string &dst_dsk_file, bool verbose,
        const std::vector<std::regex> &regexs, char default_answer,
        FileTimeAccess fileTimeAccess)
{
    auto mode = std::ios::in | std::ios::binary;
    FlexRamDisk src{src_dsk_file, mode, fileTimeAccess};
    mode |= std::ios::out;
    FlexRamDisk dst{dst_dsk_file, mode, fileTimeAccess};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath());
    }
    if (!dst.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, dst.GetPath());
    }

    size_t count = 0;
    size_t random_count = 0;
    size_t byte_size = 0;
    size_t errors = 0;

    auto matchedFilenames = GetMatchingFilenames(src, regexs);

    for (auto &filename : matchedFilenames)
    {
        bool isSuccess = true;

        try
        {
            FlexDirEntry dirEntry;

            if (dst.FindFile(filename, dirEntry))
            {
                std::string question(filename);

                question += " already exists. Overwrite?";
                if (flx::askForInput(question, "yn", default_answer))
                {
                    dst.DeleteFile(filename);
                }
                else
                {
                    if (default_answer != '?')
                    {
                        std::cout << filename << " already exists. Skipped.\n";
                    }
                    continue;
                }
            }

            src.FileCopy(filename, filename, dst);

            ++count;
            if (src.FindFile(filename, dirEntry))
            {
                byte_size += dirEntry.GetFileSize();
                random_count += dirEntry.IsRandom() ? 1 : 0;
            }
        }
        catch (FlexException &ex)
        {
            isSuccess = false;
            ++errors;
            std::cerr << "*** Error: " << ex.what() << ".\n";
            if (ex.GetErrorCode() == FERR_DISK_FULL_WRITING)
            {
                std::cerr << "    Copying aborted.\n";
                break;
            }

            std::cerr << "    Copying of " << filename << " aborted.\n";
        }

        if (isSuccess && verbose)
        {
            std::cout << "Copying " << filename << " ... Ok\n";
        }
    }

    if (verbose)
    {
        std::cout <<
            count << " file(s), " << random_count << " random file(s), ";
        if (errors != 0)
        {
            std::cout << errors << " errors, ";
        }
        auto kbyte_size = byte_size / 1024;
        std::cout << "total size: " << kbyte_size << " KByte.\n";
    }

    return 0;
}


static void helpOnDiskSize()
{
    std::cout <<
        "The following FLEX disk size parameters are supported:\n\n" <<
        "size     description\n";

    static_assert(flex_format_descriptions.size() ==
                  flex_format_shortcuts.size(),
                  "flex_format_descriptions and flex_format_shortcuts "
                  "have different size");

    for (size_t i = 0; i < flex_format_descriptions.size(); ++i)
    {
        std::cout << flex_format_shortcuts[i] << ":  " <<
                     flex_format_descriptions[i] << "\n";
    }
}

static void version()
{
    flx::print_versions(std::cout, "dsktool");
}

static void usage()
{
    std::cout <<
        "Usage: dsktool -c <dsk-file> [-v][-D] [<dsk-file>...]\n"
        "Usage: dsktool -C <dsk-file> -T<tgt-dsk-file> [-v][-z][-y|-n][-m]"
        "[-R<file>...]\n"
        "                  [<regex>...]\n"
        "Usage: dsktool -f <dsk-file> [-v][-F(dsk|flx)][-y|-n] -S<size>\n"
        "                  -B<boot-sector-file>\n"
        "Usage: dsktool -h\n"
        "Usage: dsktool -i <dsk-file> [-v][-t][-z][-y|-n] <file> [<file>...]\n"
        "Usage: dsktool -l <dsk-file> [-z][<dsk-file>...]\n"
        "Usage: dsktool -L <dsk-file> [-z][-m][-R<file>...][<regex>...]\n"
        "Usage: dsktool -r <dsk-file> [-v][-y|-n][-m][-R<file>...][<regex>...]"
        "\n"
        "Usage: dsktool -s <dsk-file> [-v] [<dsk-file>...]\n"
        "Usage: dsktool -S help\n"
        "Usage: dsktool -V\n"
        "Usage: dsktool -x <dsk-file> [-d<directory>][-t][-v][-z][-m]"
        "[-y|-n][-R<file>...]\n"
        "                  [<regex>...]\n"
        "Usage: dsktool -X <dsk-file> [-d<directory>][-t][-v][-z] "
        "[-y|-n][<dsk-file>...]\n\n"
        "Commands:\n"
        "  -c: Check consistency of FLEX disk image file.\n"
        "  -C: Copy files from a FLEX disk image file into another one.\n"
        "      If no regex is specified, all files are copied.\n"
        "  -f: Create a new FLEX disk image file.\n"
        "  -h: Print this help.\n"
        "  -i: Inject FLEX-files to a FLEX disk image file.\n"
        "  -l: List directory contents of FLEX disk image file(s).\n"
        "  -L: List directory contents of a FLEX disk image file using regex."
        "\n"
        "      If no regex is specified, all files are listed.\n"
        "  -r: Delete files from a FLEX disk image file using regex.\n"
        "  -s: One line summary of a FLEX disk image file.\n"
        "  -V: Print version number and exit.\n"
        "  -x: Extract files from a FLEX disk image file using regex.\n"
        "      If no regex is specified, all files are extracted.\n"
        "  -X: Extract all files from FLEX disk image file(s).\n\n"
        "Parameters:\n"
        "  -d<directory> The target directory.\n"
        "                Default: current directory.\n"
        "  -D            Additional debug output.\n"
        "  -F(dsk|flx)   Use *.dsk or *.flx disk image file format.\n"
        "                *.wta extension is handled as *.dsk format.\n"
        "                If not set it is determined from the file extension\n"
        "                or finally the default is *.dsk\n"
        "  -m            Regex is case sensitive (case has meaning).\n"
        "  -n            Answer no to all questions.\n"
        "  -t            Automatic detection and conversion of text files.\n"
        "  -v            More verbose output.\n"
        "  -y            Answer yes to all questions.\n"
        "  -z            Use file time (FLEX extension).\n"
        "  -R<file>      A file containing regular expressions, one per line.\n"
        "  -S<size>      Size of FLEX disk image file. Use -S help for help.\n"
        "  -B<boot-sector-file> Read contents of boot sector(s) from file.\n" <<
        "                It has a size of one or two sectors"
        " (" << SECTOR_SIZE << " or " << 2*SECTOR_SIZE << " Byte).\n"
        "  -T<dsk-file>  A target FLEX disk image file with *.dsk or *.flx "
        "format.\n"
        "                *.wta extension is handled as *.dsk format.\n"
        "  <dsk-file>    A FLEX disk image file with *.dsk or *.flx "
        "format.\n"
        "                *.wta extension is handled as *.dsk format.\n"
        "  <FLEX-file>   A FLEX text or binary file.\n"
        "  <file>        A text file or FLEX text or binary file.\n"
        "  <regex>       A regular expression specifying FLEX file(s).\n"
        "                Extended POSIX regular expression grammar is\n"
        "                supported. <regex> parameters are processed after\n"
        "                all -R<file> parameters. By default it is case\n"
        "                insensitive. See also -m.\n\n"
        "Environment Variables:\n\n"
        "  DSKTOOL_USE_FILETIME\n"
        "                Same a option -z.\n"
        "  DSKTOOL_TRACK0_ACCESS\n"
        "                Defines which sectors on track 0 are accessible.\n"
        "                If set to DIRECTORY (=default) only directory\n"
        "                sectors (within the directory sector chain) are\n"
        "                accessible. If set to FULL all sectors are\n"
        "                accessible.\n";
}

static bool getDiskTypeFromExtension(std::string ext, DiskType &disk_type)
{
    static const std::string strDsk{"dsk"};
    static const std::string strWta{"wta"};
    static const std::string strFlx{"flx"};
    ext = flx::tolower(ext);

    if (strDsk.compare(ext) == 0 || strWta.compare(ext) == 0)
    {
        disk_type = DiskType::DSK;
        return true;
    }

    if (strFlx.compare(ext) == 0)
    {
        disk_type = DiskType::FLX;
        return true;
    }

    return false;
}

static void estimateDiskType(const std::string &dsk_file, DiskType &disk_type)
{
    std::string extension;

    if (!dsk_file.empty())
    {
        extension = flx::getFileExtension(dsk_file);
        if (!extension.empty())
        {
            extension = extension.substr(1);
        }
    }

    if (!getDiskTypeFromExtension(extension, disk_type))
    {
        disk_type = DiskType::DSK;
    }
}

static bool checkDiskType(const std::string &format, DiskType &disk_type)
{
    if (getDiskTypeFromExtension(format, disk_type))
    {
        return true;
    }

    std::cerr << "*** Error: Unknown disk format '" << format << "'\n";
    return false;
}

static int checkDiskSize(const std::string &disk_size, int &tracks,
        int &sectors)
{
    static const std::string strHelp{"help"};

    if (strHelp.compare(disk_size) == 0)
    {
        helpOnDiskSize();
        return 0;
    }

    if (tracks != 0)
    {
        std::cerr << "*** Error: -S can be specified only once\n";
        return 1;
    }

    static_assert(flex_formats.size() == flex_format_shortcuts.size(),
                  "flex_formats and flex_format_shortcuts have different "
                  "size");

    for (size_t i = 0; i < flex_format_shortcuts.size(); ++i)
    {
        if (disk_size.compare(flex_format_shortcuts[i]) == 0)
        {
            tracks = flex_formats[i].trk + 1;
            sectors = flex_formats[i].sec;
            return 2;
        }
    }

    std::cerr << "*** Error: unknown disk size '" << disk_size << "'\n";
    return 1;
}

static bool checkBootSectorFile(const char *opt, const char **bsFile)
{
    struct stat sbuf{};

    if (*bsFile != nullptr)
    {
        std::cerr << "*** Error: -B can be specified only once\n";
        return false;
    }

    if (!stat(opt, &sbuf))
    {
        if (!S_ISREG(sbuf.st_mode))
        {
            std::cerr << "*** Error: " << opt <<
                         " is no regular file. Aborted.\n";
            return false;
        }

        if (sbuf.st_size != SECTOR_SIZE && sbuf.st_size != 2*SECTOR_SIZE)
        {
            std::cerr << "*** Error: Boot sector file " << opt <<
                         "\n    has to have a size of " << SECTOR_SIZE <<
                         " or " << 2*SECTOR_SIZE <<
                         " Byte (= one or two sectors). Aborted.\n";
            return false;
        }

        *bsFile = opt;
        return true;
    }

    std::cerr <<
        "*** Error: Boot sector file " << opt <<
        " not found. Aborted.\n";

    return false;
}

static char checkCommand(char oldCommand, int result)
{
    if (oldCommand == '\0')
    {
        return static_cast<char>(result);
    }

    std::cerr << "*** Error: Only one command -X, -l, -s, -c, -C, -i or -r allowed."                 "\n";
    usage();
    return 0;
}

static int checkTrack0Access()
{
    const std::string key("DSKTOOL_TRACK0_ACCESS");
    auto *track0Access = getenv(key.c_str());

    if (track0Access != nullptr)
    {
        static const std::string strDirectory{"DIRECTORY"};
        static const std::string strFull{"FULL"};

        if (strDirectory.compare(track0Access) == 0)
        {
            FlexDisk::onTrack0OnlyDirSectors = true;
        }
        else if (strFull.compare(track0Access) == 0)
        {
            FlexDisk::onTrack0OnlyDirSectors = false;
        }
        else
        {
            std::cerr << "*** Error: " << key << " has unspecified"
                         " value \"" << track0Access << "\"\n\n";
            usage();
            return 1;
        }
    }

    return 0;
}

static std::tuple<bool, std::vector<std::string> >
        readFile(const std::string &fileName)
{
    std::ifstream file(fileName);
    std::vector<std::string> lines;

    if (!file.is_open())
    {
        std::cerr << "*** Error reading from file " << fileName << "\n";
        return std::make_tuple(false, lines);
    }

    std::string line;
    while (std::getline(file, line))
    {
        lines.emplace_back(line);
    }

    return std::make_tuple(true, lines);
}

static bool addToRegexList(const std::vector<std::string> &regexLines,
        std::vector<std::regex> &regexs, bool isCaseSensitive)
{
    bool result = true;

    auto flags = std::regex_constants::extended;
    if (!isCaseSensitive)
    {
        flags |= std::regex_constants::icase;
    }

    for (const auto &regex : regexLines)
    {
        try
        {
            regexs.emplace_back(regex, flags);
        }
        catch(const std::regex_error &ex)
        {
            std::cerr <<
                "    *** Error in regex \"" << regex << "\": " << ex.what() <<
                '\n';
            result = false;
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    std::string optstr("f:X:x:L:l:s:c:C:i:r:R:T:d:o:S:F:B:DhmntvVyz");
    std::string target_dir;
    std::vector<std::string> dsk_files;
    std::vector<std::string> files;
    std::vector<std::string> regexFiles;
    std::vector<std::regex> regexs;
    std::string dsk_file;
    std::string dst_dsk_file;
    const char *bsFile = nullptr;
    DiskType disk_type{};
    bool is_disk_type_valid = false;
    int tracks = 0;
    int sectors = 0;
    bool verbose = false;
    bool debug_output = false;
    bool convert_text = false;
    bool has_regex_file = false;
    bool regexCaseSense = false;
    FileTimeAccess fileTimeAccess = FileTimeAccess::NONE;
    char default_answer = '?'; // Means: Ask user.
    int result;
    char command = '\0';
    int index;

    FlexDisk::InitializeClass();

    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'X':
            case 'l':
            case 's':
            case 'c':
                      dsk_files.emplace_back(optarg);
                      command = checkCommand(command, result);
                      if (command == '\0')
                      {
                          return 1;
                      }
                      break;

            case 'x':
            case 'f':
            case 'i':
            case 'r':
            case 'L':
            case 'C':
                      dsk_file = optarg;
                      command = checkCommand(command, result);
                      if (command == '\0')
                      {
                          return 1;
                      }
                      break;

            case 'd': target_dir = optarg;
                      break;

            case 'D': debug_output = true;
                      break;

            case 'R': has_regex_file = true;
                      if (command == 'C' || command == 'x')
                      {
                          regexFiles.emplace_back(optarg);
                      }
                      break;

            case 'T': dst_dsk_file = optarg;
                      break;

            case 'm': regexCaseSense = true;
                      break;

            case 'V': version();
                      return 0;

            case 'h': usage();
                      return 0;

            case 'n':
            case 'y':
                      if (default_answer != '?')
                      {
                          std::cerr << "*** Error: Only one of -y or -n allowed"
                                       "\n";
                          usage();
                          return 0;
                      }
                      default_answer = static_cast<char>(result);
                      break;

            case 'S':
                    {
                        const auto s = checkDiskSize(optarg, tracks, sectors);
                        if (s < 2)
                        {
                            return s;
                        }
                        break;
                    }

            case 'F': if (is_disk_type_valid)
                      {
                          std::cerr << "*** Error: -F can be specified only "
                              "once\n";
                          return 1;
                      }
                      if (!checkDiskType(optarg, disk_type))
                      {
                          return 1;
                      }
                      is_disk_type_valid = true;
                      break;

            case 'B': if (!checkBootSectorFile(optarg, &bsFile))
                      {
                          return 1;
                      }
                      break;

            case 't': convert_text = true;
                      break;

            case 'v': verbose = true;
                      break;

            case 'z': fileTimeAccess =
                          FileTimeAccess::Get | FileTimeAccess::Set;
                      break;

            case '?':
                      if (optopt != 'X' && !isprint(optopt))
                      {
                          std::cerr << "Unknown option character '\\x" <<
                                       std::hex << optopt << "'.\n";
                      }
                      return 1;
            default:  return 1;
        }
    }

    bool isRegexCommand = (command == 'C' || command == 'x' ||
                           command == 'L' || command == 'r');

    if (!is_disk_type_valid)
    {
        estimateDiskType(dsk_file, disk_type);
        is_disk_type_valid = true;
    }

    for (const auto &regexFile : regexFiles)
    {
        // Create regular expr. from regex file contents.
        bool success;
        std::vector<std::string> regexLines;
        tie(success, regexLines) = readFile(regexFile);

        if (!success || !addToRegexList(regexLines, regexs, regexCaseSense))
        {
            usage();
            return 1;
        }
    }

    if (command == 'i')
    {
        for (index = optind; index < argc; index++)
        {
            files.emplace_back(argv[index]);
        }
    }
    else if (command == 'c' || command == 'l' ||
             command == 's' || command == 'X')
    {
        for (index = optind; index < argc; index++)
        {
            dsk_files.emplace_back(argv[index]);
        }
    }
    else if (isRegexCommand)
    {
        std::vector<std::string> regexLines;

        for (index = optind; index < argc; index++)
        {
            regexLines.emplace_back(argv[index]);
        }

        if (!addToRegexList(regexLines, regexs, regexCaseSense))
        {
            usage();
            return 1;
        }
    }

    if (command == 0)
    {
        std::cerr << "*** Error: No command specified\n";
        usage();
        return 1;
    }

    if ((tolower(command == 'l') && verbose) ||
        (command == 'i' && files.empty()) ||
        (command == 'C' && dst_dsk_file.empty()) ||
        (command != 'X' && command != 'x' && !target_dir.empty()) ||
        (command != 'C' && !dst_dsk_file.empty()) ||
        (!isRegexCommand && regexCaseSense) ||
        (!isRegexCommand && has_regex_file) ||
        (!isRegexCommand && !regexs.empty()) ||
        (std::string("firCxX").find_first_of(command) == std::string::npos &&
         (default_answer != '?')) ||
        (command != 'i' && command != 'X' && command != 'x' && convert_text) ||
        (command != 'c' && debug_output) ||
        (std::string("cCilLxX").find_first_of(command) == std::string::npos &&
            (fileTimeAccess != FileTimeAccess::NONE)) ||
        (command != 'f' && bsFile != nullptr))
    {
        std::cerr << "*** Error: Wrong syntax\n";
        usage();
        return 1;
    }

    if (std::string("cCilLxX").find_first_of(command) != std::string::npos &&
       (getenv("DSKTOOL_USE_FILETIME") != nullptr))
    {
        fileTimeAccess = FileTimeAccess::Get | FileTimeAccess::Set;
    }

    result = checkTrack0Access();
    if (result != 0)
    {
        return result;
    }

    try
    {
        switch(command)
        {
            case 'c':
                return CheckConsistencyOfDskFiles(dsk_files, verbose,
                                                  debug_output,
                                                  fileTimeAccess);

            case 'f':
                return FormatFlexDiskFile(dsk_file, disk_type, tracks,
                                          sectors, default_answer, verbose,
                                          bsFile);

            case 'i':
                return InjectToDskFile(dsk_file, verbose, files,
                                       default_answer, convert_text,
                                       fileTimeAccess);

            case 'L':
                dsk_files.push_back(dsk_file);
                FALLTHROUGH;

            case 'l':
                return ListDirectoryOfDskFiles(dsk_files, regexs,
                                               fileTimeAccess);

            case 'r':
                return DeleteFromDskFile(dsk_file, verbose, regexs,
                                         default_answer);

            case 's':
                return SummaryOfDskFiles(dsk_files, verbose);

            case 'x':
                dsk_files.push_back(dsk_file);
                FALLTHROUGH;

            case 'X':
                return ExtractDskFiles(target_dir, verbose, convert_text,
                                       default_answer, dsk_files, regexs,
                                       fileTimeAccess);

            case 'C':
                return CopyFromToDskFile(dsk_file, dst_dsk_file, verbose,
                                         regexs, default_answer,
                                         fileTimeAccess);
        }
    }
    catch (FlexException &ex)
    {
        std::cerr << "   *** Error: " << ex.what() << ". Aborted.\n";
    }
}

