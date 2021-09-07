/*
    dsktool.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2021  W. Schwotzer

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
#include "flexerr.h"
#include "fdirent.h"
#include "rfilecnt.h"
#include "dircont.h"
#include "ifilecnt.h"
#include "fcopyman.h"
#include "fcinfo.h"
#include "filfschk.h"
#include "ffilebuf.h"
#include "filecnts.h"


std::vector<std::string> GetMatchingFilenames(
    FlexFileContainer &container,
    const std::vector<std::regex> &regexs)
{
    FileContainerIterator iter;
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

int FormatFlexDiskFile(const std::string &dsk_file, int disk_format,
                       int tracks, int sectors, char default_answer,
                       bool verbose)
{
    struct stat sbuf;

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
        if (AskForInput(question, "yn", default_answer))
        {
            unlink(dsk_file.c_str());
        }
        else
        {
            if (verbose)
            {
                std::cout << dsk_file << " already exists. Skipped.\n";
            }
            return 0;
        }
    }

    try
    {
        std::unique_ptr<FlexFileContainer> container;

        container.reset(FlexFileContainer::Create(
                        getParentPath(dsk_file).c_str(),
                        getFileName(dsk_file).c_str(),
                        tracks, sectors, disk_format));

        if (container && verbose)
        {
            std::cout << "Successfully created " <<
                getFileName(dsk_file) << " with " <<
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

int ExtractDskFile(const std::string &target_dir, bool verbose,
                   bool convert_text, const std::string &dsk_file,
                   const std::vector<std::regex> &regexs)
{
    std::string subdir = target_dir + getFileStem(getFileName(dsk_file));

    if (verbose)
    {
        std::cout << "Extracting " << dsk_file << " into " << subdir <<
                     " ... " << std::endl;
    }

    if (regexs.empty() && BDirectory::Exists(subdir))
    {
        std::cerr << "  *** Error: '" << subdir << "' already exists.\n" <<
                     "    Extraction from '" << dsk_file << "' aborted.\n";
        return 0;
    }

    // If there are regular expressions subdir may already exist.
    // If subdir does not exist create it.
    if ((regexs.empty() ||
        (!regexs.empty() && !BDirectory::Exists(subdir))) &&
        !BDirectory::Create(subdir))
    {
        std::cerr << "  *** Error creating '" << subdir << "'.\n" <<
                     "    Extraction from '" << dsk_file << "' aborted.\n";
        return 0;
    }

    FlexCopyManager::autoTextConversion = convert_text;
    FlexRamFileContainer src{dsk_file.c_str(), "rb"};
    DirectoryContainer dest{subdir.c_str()};
    size_t count = 0;
    size_t random_count = 0;
    size_t byte_size = 0;
    size_t errors = 0;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }

    auto matchedFilenames = GetMatchingFilenames(src, regexs);

    for (const auto &filename : matchedFilenames)
    {
        std::string result = "ok";
        std::string what;

        try
        {
            FlexDirEntry dir_entry;

            src.FileCopy(filename.c_str(), filename.c_str(), dest);

            ++count;
            if (src.FindFile(filename.c_str(), dir_entry))
            {
                byte_size += dir_entry.GetFileSize();
                random_count += dir_entry.IsRandom() ? 1 : 0;
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
            std::cout << " extracting " << filename << " ... " << result <<
                         std::endl;
        }
        if (!what.empty())
        {
            std::cerr << "   *** Error: " << what << std::endl;
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
        auto kbyte_size = byte_size / 1024.0;
        std::cout << "total size: " << kbyte_size << " KByte." << std::endl; 
    }

    return 0;
}

int ExtractDskFiles(std::string target_dir, bool verbose, bool convert_text,
                    const std::vector<std::string> &dsk_files,
                    const std::vector<std::regex> &regexs)
{
    if (target_dir.empty())
    {
        target_dir = ".";
    }

    if (!BDirectory::Exists(target_dir))
    {
        std::cerr << "*** Error: '" << target_dir << "' does not exist or is"
        " no directory.\n";
        return 1;
    }

    if (target_dir.at(target_dir.size() - 1) != PATHSEPARATOR)
    {
        target_dir += PATHSEPARATORSTRING;
    }

    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            int result = ExtractDskFile(target_dir, verbose, convert_text,
                                        dsk_file, regexs);
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
                std::cout << " failed." << std::endl;
            }
            std::cerr <<
                "   *** Error: " << ex.what() << ".\n" <<
                "       Extraction from '" << dsk_file << "' aborted.\n";
        }
    }

    return 0;
}

int ListDirectoryOfDskFile(const std::string &dsk_file)
{
    FlexRamFileContainer src{dsk_file.c_str(), "rb"};
    FileContainerIterator iter;
    FlexContainerInfo info;
    unsigned int number = 0;
    bool hasInfo = false;
    int sumSectors = 0;
    int largest = 0;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }

    if (src.GetInfo(info))
    {
        hasInfo = true;
        std::cout <<
            "FILE: " << getFileName(dsk_file) << "  " <<
            "DISK: " << info.GetName() <<
            " #" << info.GetNumber() <<
            "  CREATED: " << info.GetDate().GetDateString() <<
            "\n";
    }
    else
    {
        std::cerr << "Error reading Container info " <<
                     getFileName(dsk_file) << "\n";
    }

    std::cout << "FILE#   NAME   TYPE  BEGIN   END   SIZE    DATE     " <<
                 "PRT   RND" << std::endl << std::endl;

    for (iter = src.begin(); iter != src.end(); ++iter)
    {
        const auto &dir_entry = *iter;
        int startTrack, startSector;
        int endTrack, endSector;

        ++number;
        dir_entry.GetStartTrkSec(startTrack, startSector);
        dir_entry.GetEndTrkSec(endTrack, endSector);

        int sectors = static_cast<int>(dir_entry.GetFileSize() / SECTOR_SIZE);
        sumSectors += sectors;
        if (sectors > largest)
        {
            largest = sectors;
        }

        std::cout <<
            std::right << std::setw(5) << number << "  " <<
            std::left << std::setw(8) << dir_entry.GetFileName() << "." <<
            std::left << std::setw(3) << dir_entry.GetFileExt() << "  " <<
            std::right << std::uppercase << std::hex << std::setfill('0') <<
            std::setw(2) << startTrack << "-" <<
            std::setw(2) << startSector << "  " <<
            std::setw(2) << endTrack << "-" <<
            std::setw(2) << endSector << " " <<
            std::dec << std::setfill(' ') <<
            std::setw(5) << sectors << "  " <<
            std::setw(11) << dir_entry.GetDate().GetDateString() << " " <<
            std::left << std::setw(4) <<
            dir_entry.GetAttributesString() << " " <<
            (dir_entry.IsRandom() ? "R" : "") << std::endl;
    }

    if (hasInfo)
    {
        std::cout << std::endl << "    " <<
                     "FILES=" << number <<
                     ", SECTORS=" << sumSectors <<
                     ", LARGEST=" << largest <<
                     ", FREE=" << (info.GetFree() / SECTOR_SIZE) <<
                     std::endl << std::endl;
    }

    return 0;
}

int ListDirectoryOfDskFiles(const std::vector<std::string> &dsk_files)
{
    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            ListDirectoryOfDskFile(dsk_file);
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

int SummaryOfDskFile(const std::string &dsk_file,
                     size_t &sum_files, size_t &sum_size, size_t &sum_free,
                     bool verbose)
{
    FlexRamFileContainer src{dsk_file.c_str(), "rb"};
    FileContainerIterator iter;
    FlexContainerInfo info;

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }

    if (src.GetInfo(info))
    {
        size_t file_count = 0;
        int tracks;
        int sectors;

        info.GetTrackSector(tracks, sectors);

        for (iter = src.begin(); iter != src.end(); ++iter)
        {
            ++file_count;
        }
        sum_files += file_count;

        auto name = info.GetName();
        if (name.empty())
        {
            name = "\"\"";
        }

        std::string file = verbose ? dsk_file : getFileName(dsk_file);

        std::cout <<
            std::left <<
            info.GetDate().GetDateString() << " " <<
            std::setw(12) << name << " " <<
            std::setw(5) << info.GetNumber() << " " <<
            std::setw(2) << tracks << "-" << std::setw(2) << sectors << " " <<
            std::setw(5) << file_count << " " <<
            std::setw(5) << (info.GetTotalSize() / SECTOR_SIZE) << " " <<
            std::setw(5) << (info.GetFree() / SECTOR_SIZE) << " " <<
            file << "\n";
        sum_size += (info.GetTotalSize() / SECTOR_SIZE);
        sum_free += (info.GetFree() / SECTOR_SIZE);
    }
    else
    {
        std::cerr << "Error reading Container info for " <<
                     getFileName(dsk_file) << "\n";
    }

    return 0;
}

int SummaryOfDskFiles(const std::vector<std::string> &dsk_files, bool verbose)
{
    size_t sum_files = 0;
    size_t sum_size = 0;
    size_t sum_free = 0;

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
            (sum_size / 4.0) << " KByte" <<
            " FREE: " << sum_free << " Sectors/" <<
            (sum_free / 4.0) << " KByte\n";
    }

    return 0;
}

int InjectToDskFile(const std::string &dsk_file, bool verbose,
                    const std::vector<std::string> &files,
                    char default_answer, bool isConvertText)
{
    FlexRamFileContainer dst{dsk_file.c_str(), "rb+"};

    if (!dst.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, dst.GetPath().c_str());
    }

    for (auto &file : files)
    {
        bool isSuccess = true;
        FlexFileBuffer fileBuffer;

        if (!fileBuffer.ReadFromFile(file.c_str()))
        {
            std::cerr <<
                "   *** Error: Reading from " << file << ". Aborted.\n";
            continue;
        }

        if (isConvertText && fileBuffer.IsTextFile())
        {
            fileBuffer.ConvertToFlexTextFile();
        }

        try
        {
            FlexDirEntry dir_entry;

            if (dst.FindFile(fileBuffer.GetFilename(), dir_entry))
            {
                std::string question(fileBuffer.GetFilename());

                question += " already exists. Overwrite?";
                if (AskForInput(question, "yn", default_answer))
                {
                    dst.DeleteFile(dir_entry.GetTotalFileName().c_str());
                }
                else
                {
                    if (verbose)
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
            std::cout << "Injecting " << file << " ... Ok\n";
        }
    }

    return 0;
}

int DeleteFromDskFile(const std::string &dsk_file, bool verbose,
                      const std::vector<std::string> &flex_files,
                      char default_answer)
{
    FlexRamFileContainer src{dsk_file.c_str(), "rb+"};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }

    for (auto &flex_file : flex_files)
    {
        bool isSuccess = true;

        try
        {
            FlexDirEntry dir_entry;

            if (src.FindFile(flex_file.c_str(), dir_entry))
            {
                std::stringstream question;

                question << "Delete " << flex_file << "?";
                if (AskForInput(question.str(), "yn", default_answer))
                {
                    src.DeleteFile(flex_file.c_str());
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

int CheckConsistencyOfDskFile(const std::string &dsk_file, bool verbose,
                              bool debug_output)
{
    FlexRamFileContainer src{dsk_file.c_str(), "rb"};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }

    FileContainerCheck check(src, verbose);

    std::cout << "Check " << dsk_file << " ...";
    if (check.CheckFileSystem())
    {
        std::cout << " Ok" << std::endl;
    }
    else
    {
        std::string separator;
        size_t infos = 0;
        size_t warnings = 0;
        size_t errors = 0;
        const auto &results = check.GetResult();

        for (const auto &result : results)
        {
            switch (result->type)
            {
                case ContainerCheckResultItem::Type::Info:
                    ++infos;
                    break;
                case ContainerCheckResultItem::Type::Warning:
                    ++warnings;
                    break;
                case ContainerCheckResultItem::Type::Error:
                    ++errors;
                    break;
            }
        }

        if (errors != 0)
        {
            std::cout << " " << errors << " error(s)";
            separator = ",";
        }
        if (warnings != 0)
        {
            std::cout << separator << " " << warnings << " warning(s)";
            separator = " and";
        }
        if (infos != 0)
        {
            std::cout << separator << " " << infos << " info(s)";
        }
        std::cout << std::endl;

        if (verbose)
        {
            for (const auto &result : results)
            {
                std::cout << "  " << result << std::endl;
            }
        }
    }

    if (debug_output)
    {
        check.DebugDump(std::cerr);
    }

    return 0;
}

int CheckConsistencyOfDskFiles(const std::vector<std::string> &dsk_files,
                               bool verbose, bool debug_output)
{
    for (const auto &dsk_file : dsk_files)
    {
        try
        {
            CheckConsistencyOfDskFile(dsk_file, verbose, debug_output);
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

int CopyFromToDskFile(const std::string &src_dsk_file,
        const std::string &dst_dsk_file, bool verbose,
        const std::vector<std::regex> &regexs, char default_answer)
{
    FlexRamFileContainer src{src_dsk_file.c_str(), "rb"};
    FlexRamFileContainer dst{dst_dsk_file.c_str(), "rb+"};

    if (!src.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, src.GetPath().c_str());
    }
    if (!dst.IsFlexFormat())
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, dst.GetPath().c_str());
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
            FlexDirEntry dir_entry;

            if (dst.FindFile(filename.c_str(), dir_entry))
            {
                std::string question(filename);

                question += " already exists. Overwrite?";
                if (AskForInput(question, "yn", default_answer))
                {
                    dst.DeleteFile(filename.c_str());
                }
                else
                {
                    if (verbose)
                    {
                        std::cout << filename << " already exists. Skipped.\n";
                    }
                    continue;
                }
            }

            src.FileCopy(filename.c_str(), filename.c_str(), dst);

            ++count;
            if (src.FindFile(filename.c_str(), dir_entry))
            {
                byte_size += dir_entry.GetFileSize();
                random_count += dir_entry.IsRandom() ? 1 : 0;
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
            else
            {
                std::cerr << "    Copying of " << filename << " aborted.\n";
            }
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
        auto kbyte_size = byte_size / 1024.0;
        std::cout << "total size: " << kbyte_size << " KByte." << std::endl;
    }

    return 0;
}


void helpOnDiskSize()
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

void version()
{
    std::cout <<
        "dsktool " << VERSION << "\n" <<
        "dsktool " << COPYRIGHT_MESSAGE;
}

void usage()
{
    std::cout <<
        "Usage: dsktool -c <dsk-file> [-v][-D] [<dsk-file>...]\n" <<
        "Usage: dsktool -C <dsk-file> -T<tgt-dsk-file> [-v][-y|-n][-R<file>...]"
        "[<regex>...]\n" <<
        "Usage: dsktool -f <dsk-file> [-v][-F(dsk|flx)][-y|-n] -S<size>\n" <<
        "Usage: dsktool -h\n" <<
        "Usage: dsktool -i <dsk-file> [-v][-t][-y|-n] <file> [<file>...]\n" <<
        "Usage: dsktool -l <dsk-file> [<dsk-file>...]\n" <<
        "Usage: dsktool -r <dsk-file> [-v][-y|-n] <FLEX-file> "
        "[<FLEX-file>...]\n" <<
        "Usage: dsktool -s <dsk-file> [-v] [<dsk-file>...]\n" <<
        "Usage: dsktool -S help\n" <<
        "Usage: dsktool -V\n" <<
        "Usage: dsktool -x <dsk-file> [-d<directory>][-t][-v][-R<file>...]"
        "[<regex>...]\n" <<
        "Usage: dsktool -X <dsk-file> [-d<directory>][-t][-v] "
        "[<dsk-file>...]\n\n" <<
        "Commands:\n" <<
        "  -c: Check consistency of FLEX file container.\n" <<
        "  -C: Copy files from a FLEX file container into another one.\n" <<
        "      If no regex is specified, all files are copied.\n" <<
        "  -f: Create a new FLEX file container.\n" <<
        "  -h: Print this help.\n" <<
        "  -i: Inject FLEX-files to a FLEX file container.\n" <<
        "  -l: List directory contents of a FLEX file container.\n" <<
        "  -r: Delete FLEX-files from a FLEX file container.\n" <<
        "  -s: One line summary of a FLEX file container.\n" <<
        "  -V: Print version number and exit.\n" <<
        "  -x: Extract files from a FLEX file container using regex.\n" <<
        "      If no regex is specified, all files are extracted.\n" <<
        "  -X: Extract all files from FLEX file container(s).\n\n" <<
        "Parameters:\n" <<
        "  -d<directory> The target directory.\n" <<
        "                For each dsk-file a subdirectory is created.\n" <<
        "                Default: current directory.\n" <<
        "  -D            Additional debug output.\n" <<
        "  -F(dsk|flx)   Use *.dsk or *.flx file container format.\n" <<
        "                If not set it is determined from the file extension"
        "\n" <<
        "                or finally the default is *.dsk\n" <<
        "  -n            Answer no to all questions.\n" <<
        "  -t            Automatic detection and conversion of text files.\n" <<
        "  -v            More verbose output.\n" <<
        "  -y            Answer yes to all questions.\n" <<
        "  -R<file>      A file containing regular expressions, one per line."
        "\n" <<
        "  -S<size>      Size of FLEX file container. Use -S help for help."
        "\n" <<
        "  -T<dsk-file>  A target FLEX file container with *.dsk or *.flx "
        "format.\n" <<
        "  <dsk-file>    A FLEX file container with *.dsk or *.flx format.\n" <<
        "  <FLEX-file>   A FLEX text or binary file.\n" <<
        "  <file>        A text file or FLEX text or binary file.\n" <<
        "  <regex>       A regular expression specifying FLEX file(s).\n" <<
        "                Extended POSIX regular expression grammar is\n" <<
        "                supported. <regex> parameters are processed after\n" <<
        "                all -R<file> parameters.\n";
}

bool estimateDiskFormat(const char *format, int &disk_format)
{
    if (format != nullptr)
    {
        if (strcmp(format, "dsk") == 0)
        {
            disk_format = TYPE_DSK_CONTAINER;
            return true;
        }
        else if (strcmp(format, "flx") == 0)
        {
            disk_format = TYPE_FLX_CONTAINER;
            return true;
        }
    }

    return false;
}

void estimateDiskFormat(const std::string &dsk_file, int &disk_format)
{
    std::string extension;

    if (!dsk_file.empty())
    { 
        extension = getFileExtension(dsk_file);
        if (!extension.empty())
        {
            extension = extension.substr(1);
        }
    }

    strlower(extension);
    if (!estimateDiskFormat(extension.c_str(), disk_format))
    {
        disk_format = TYPE_DSK_CONTAINER;
    }
}

bool checkDiskFormat(const char *format, int &disk_format)
{
    if (disk_format != 0)
    {
        std::cerr << "*** Error: -F can be specified only once\n";
        return false;
    }

    if (format != nullptr && estimateDiskFormat(format, disk_format))
    {
        return true;
    }

    std::cerr << "*** Error: Unknown disk format '" << format << "'\n";
    return false;
}

int checkDiskSize(const char *disk_size, int &tracks, int &sectors)
{
    if (disk_size != nullptr)
    {
        if (strcmp(disk_size, "help") == 0)
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
            if (strcmp(disk_size, flex_format_shortcuts[i]) == 0)
            {
                tracks = flex_formats[i].trk;
                sectors = flex_formats[i].sec;
                return 2;
            }
        }
    }

    std::cerr << "*** Error: unknown disk size '" << disk_size << "'\n";
    return 1;
}

char checkCommand(char oldCommand, int result)
{
    if (oldCommand == '\0')
    {
        return static_cast<char>(result);
    }

    std::cerr << "*** Error: Only one command -X, -l, -s, -c, -C, -i or -r allowed."                 "\n";
    usage();
    return 0;
}

std::tuple<bool, std::vector<std::string> >
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

bool addToRegexList(const std::vector<std::string> &regexLines,
                    std::vector<std::regex> &regexs)
{
    bool result = true;

    const auto flags = std::regex_constants::extended;

    for (const auto &regex : regexLines)
    {
        try
        {
            regexs.push_back(std::regex(regex, flags));
        }
        catch(const std::regex_error &ex)
        {
            std::cerr <<
                "    *** Error in regex \"" << regex << "\": " << ex.what() <<
                std::endl;
            result = false;
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    std::string optstr("f:X:x:l:s:c:C:i:r:R:T:d:o:S:F:DhntvVy");
    std::string target_dir;
    std::vector<std::string> dsk_files;
    std::vector<std::string> files;
    std::vector<std::regex> regexs;
    std::string dsk_file;
    std::string dst_dsk_file;
    std::string size;
    int disk_format = 0;
    int tracks = 0;
    int sectors = 0;
    bool verbose = false;
    bool debug_output = false;
    bool convert_text = false;
    bool has_regex_file = false;
    char default_answer = '?'; // Means: Ask user.
    int result;
    char command = '\0';
    int index;

    while ((result = getopt(argc, argv, optstr.c_str())) != -1)
    {
        switch (result)
        {
            case 'X':
            case 'l':
            case 's':
            case 'c':
                      dsk_files.push_back(optarg);
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
                          // Create regular expr. from regex file contents.
                          bool success;
                          std::vector<std::string> regexLines;
                          tie(success, regexLines) = readFile(optarg);

                          if (!success || !addToRegexList(regexLines, regexs))
                          {
                              usage();
                              return 1;
                          }
                      }
                      break;

            case 'T': dst_dsk_file = optarg;
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
                        int s;

                        if ((s = checkDiskSize(optarg, tracks, sectors)) < 2)
                        {
                            return s;
                        }
                        break;
                    }

            case 'F': if (!checkDiskFormat(optarg, disk_format))
                      {
                          return 1;
                      }
                      break;

            case 't': convert_text = true;
                      break;

            case 'v': verbose = true;
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

    if (disk_format == 0)
    {
        estimateDiskFormat(dsk_file, disk_format);
    }

    if (command == 'i' || command == 'r')
    {
        for (index = optind; index < argc; index++)
        {
            files.push_back(argv[index]);
        }
    }
    else if (command == 'c' || command == 'l' ||
             command == 's' || command == 'X')
    {
        for (index = optind; index < argc; index++)
        {
            dsk_files.push_back(argv[index]);
        }
    }
    else if (command == 'C' || command == 'x')
    {
        std::vector<std::string> regexLines;

        for (index = optind; index < argc; index++)
        {
            regexLines.push_back(argv[index]);
        }

        if (!addToRegexList(regexLines, regexs))
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

    if ((command == 'l' && verbose) ||
        (command == 'i' && files.empty()) ||
        (command == 'r' && files.empty()) ||
        (command == 'C' && dst_dsk_file.empty()) ||
        (command != 'X' && command != 'x' && !target_dir.empty()) ||
        (command != 'C' && !dst_dsk_file.empty()) ||
        (command != 'C' && command != 'x' && has_regex_file) ||
        (command != 'C' && command != 'x' && !regexs.empty()) ||
        (std::string("firC").find_first_of(command) == std::string::npos &&
         (default_answer != '?')) ||
        (command != 'i' && command != 'X' && command != 'x' && convert_text) ||
        (command != 'c' && debug_output))
    {
        std::cerr << "*** Error: Wrong syntax\n";
        usage();
        return 1;
    }

    try
    {
        switch(command)
        {
            case 'c':
                return CheckConsistencyOfDskFiles(dsk_files, verbose,
                                                  debug_output);

            case 'f':
                return FormatFlexDiskFile(dsk_file, disk_format, tracks,
                                          sectors, default_answer, verbose);

            case 'i':
                return InjectToDskFile(dsk_file, verbose, files,
                                       default_answer, convert_text);

            case 'l':
                return ListDirectoryOfDskFiles(dsk_files);

            case 'r':
                return DeleteFromDskFile(dsk_file, verbose, files,
                                         default_answer);

            case 's':
                return SummaryOfDskFiles(dsk_files, verbose);

            case 'x':
                dsk_files.push_back(dsk_file);
                // fall through
            case 'X':
                return ExtractDskFiles(target_dir, verbose, convert_text,
                                       dsk_files, regexs);

            case 'C':
                return CopyFromToDskFile(dsk_file, dst_dsk_file, verbose,
                                         regexs, default_answer);
        }
    }
    catch (FlexException &ex)
    {
        std::cerr << "   *** Error: " << ex.what() << ". Aborted.\n";
    }
}

