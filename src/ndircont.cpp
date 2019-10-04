/*
    ndircont.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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

#ifdef NAFS
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <locale>
#include <stdio.h>
#include <sys/stat.h>
#include <new>          // needed for (nothrow)
#include <ctype.h>
#include <limits.h>

#include "bdir.h"
#include "bfileptr.h"
#include "ndircont.h"
#include "fdirent.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "cvtwchar.h"

// detailed debug messegas can be written to a debug file:
//#define DEBUG_FILE "debug.txt"
#ifdef DEBUG_FILE
    #include "debug.h"
#endif


NafsDirectoryContainer::NafsDirectoryContainer(const char *path) :
    attributes(0), isOpen(false), dir_sectors(0), new_files(0), dir_extend{0}
{
    struct stat sbuf;
    static Word number = 0;

    if (path == nullptr || stat(path, &sbuf) || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    directory = path;
    isOpen = true;

    if (access(path, W_OK))
    {
        attributes |= FLX_READONLY;
    }

    mount(number);
    number++;
}

NafsDirectoryContainer::~NafsDirectoryContainer()
{
    // final cleanup: close if not already done
    try
    {
        Close();
    }
    catch (...)
    {
        // ignore exceptions
        // exceptions in destructors cause much problems
        // usually the file should be closed already
    }
}

// type, track and sectors parameter will be ignored
NafsDirectoryContainer *NafsDirectoryContainer::Create(const char *pdir,
        const char *name,
        int /* = 0 */,
        int /* = 0 */,
        int /* = TYPE_DSK_FORMAT */)
{
    struct stat sbuf;
    std::string totalPath;

    if (pdir == nullptr || stat(pdir, &sbuf) || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_CREATE, name);
    }

    totalPath = pdir;
    totalPath += PATHSEPARATORSTRING;
    totalPath += name;

    if (!BDirectory::Create(totalPath, 0755))
    {
        throw FlexException(FERR_UNABLE_TO_CREATE, name);
    }

    return new NafsDirectoryContainer(totalPath.c_str());
}

bool NafsDirectoryContainer::IsContainerOpened() const
{
    return isOpen;
}

std::string NafsDirectoryContainer::GetPath() const
{
    if (isOpen)
    {
        return directory;
    }

    return "";
}

bool NafsDirectoryContainer::Close()
{
    if (isOpen)
    {
        close_new_files();
        free_memory();
        directory.clear();
        isOpen = false;
    }

    return true;
}

bool NafsDirectoryContainer::GetInfo(FlexContainerInfo &info) const
{
    struct s_sys_info_sector buffer;

    if (!isOpen)
    {
        return false;
    }

    if (!ReadSector((Byte *)&buffer, 0, 3))
    {
        throw FlexException(FERR_READING_TRKSEC, 0, 3, directory.c_str());
    }

    info.SetDate(buffer.day, buffer.month, buffer.year);
    info.SetTrackSector(buffer.last_trk + 1, buffer.last_sec);
    info.SetFree((((buffer.free[0] << 8) | buffer.free[1]) *
                  param.byte_p_sector) >> 10);
    info.SetTotalSize(((buffer.last_sec * (buffer.last_trk + 1)) *
                       param.byte_p_sector) >> 10);
    info.SetName(buffer.disk_name);
    info.SetPath(directory.c_str());
    info.SetType(param.type);
    info.SetAttributes(attributes);
    return true;
}

int NafsDirectoryContainer::GetBytesPerSector() const
{
    return param.byte_p_sector;
}

bool NafsDirectoryContainer::IsWriteProtected() const
{
    return (param.write_protect) ? true : false;
}

bool NafsDirectoryContainer::IsTrackValid(int track) const
{
    return (track <= param.max_track);
}

bool NafsDirectoryContainer::IsSectorValid(int track, int sector) const
{
    if (track)
    {
        return (sector != 0 && sector <= (param.max_sector * 2));
    }
    else
    {
        return (sector != 0 && sector <= (param.max_sector0 * 2));
    }
}

int NafsDirectoryContainer::GetContainerType() const
{
    return param.type;
}

///////////////////////////////////////////////////////
// private interface
///////////////////////////////////////////////////////

void NafsDirectoryContainer::initialize_header(Byte wp)
{
    size_t i;

    param.offset        = 0;
    param.write_protect = wp;
    param.max_sector    = MAX_SECTOR >> 1;
    param.max_sector0   = (INIT_DIR_SECTORS + 4) >> 1;
    param.max_track     = MAX_TRACK;
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector0 << 9;
    param.byte_p_track  = param.max_sector << 9;
    param.type = TYPE_DIRECTORY | TYPE_NAFS_DIRECTORY;

    pflex_links = std::unique_ptr<s_link_table[]>(
            new s_link_table[LINK_TABLE_SIZE]);
    check_pointer(pflex_links.get());
    for (i = 0; i < LINK_TABLE_SIZE; ++i)
    {
        pflex_links[i] = { };
    }

    pflex_sys_info = std::unique_ptr<s_sys_info_sector[]>(
            new s_sys_info_sector[2]);
    check_pointer(pflex_sys_info.get());
    for (i = 0; i < 2; ++i)
    {
        pflex_sys_info[i] = { };
    }

    pflex_unused = std::unique_ptr<s_unused_sector>(new s_unused_sector);
    check_pointer(pflex_unused.get());
    *pflex_unused = { };

    dir_sectors = INIT_DIR_SECTORS;
    pflex_directory = std::unique_ptr<s_dir_sector[]>(
            new s_dir_sector[dir_sectors]);
    check_pointer(pflex_directory.get());
    for (i = 0; i < dir_sectors; ++i)
    {
        pflex_directory[i] = { };
    }

    new_files = INIT_NEW_FILES;
    pnew_file = std::unique_ptr<s_new_file[]>(new s_new_file[new_files]);
    check_pointer(pnew_file.get());
    for (i = 0; i < new_files; ++i)
    {
        pnew_file[i] = { };
    }
    dir_extend.sec_trk = 0;
} // initialize_header

void NafsDirectoryContainer::check_pointer(void *ptr)
{
    if (ptr == nullptr)
    {
        std::string msg("Not enough memory, can't continue\n");

#ifdef _WIN32
        std::string title(PROGRAMNAME " error");
#ifdef UNICODE
        MessageBox(nullptr, ConvertToUtf16String(msg).c_str(),
            ConvertToUtf16String(title).c_str(),
            MB_OK | MB_ICONERROR);
#else
        MessageBox(nullptr, msg.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
#endif
#endif
#ifdef UNIX
        fprintf(stderr, "%s", msg.c_str());
#endif
        exit(EXIT_FAILURE);
    } // if
}

// if valid flex filename return is > 0 otherwise = 0
/*
    some examples:

    allowed:        x.a xx.a xxxxxxxx.a x xx xxxxxxxx
    not allowed:    x. .a xx. xxxxxxxxx.a X.a xxxxxxxxX.a
*/
bool NafsDirectoryContainer::IsFlexFilename(const char *pfilename,
        char *pname /* = nullptr */,
        char *pext  /* = nullptr */) const
{
    int result; // result from sscanf should be int
    char dot;
    char name[9];
    char ext[4];
    const char *format;

    dot    = '\0';
    format = "%1[A-Za-z]%7[A-Za-z0-9_-]";
    result = sscanf(pfilename, format, name, &name[1]);

    if (!result || result == EOF)
    {
        return false;
    }

    if (result == 1)
    {
        format = "%*1[A-Za-z]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }
    else
    {
        format = "%*1[A-Za-z]%*7[A-Za-z0-9_-]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }

    result = sscanf(pfilename, format, &dot, ext, &ext[1]);

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != strlen(pfilename))
    {
        return false;
    }

    strupper(name);
    strupper(ext);

    if (pname)
    {
        strcpy(pname, name);
    }

    if (pext)
    {
        strcpy(pext, ext);
    }

    return true;
} // IsFlexFilename


void NafsDirectoryContainer::initialize_flex_directory()
{
    Word i;

    for (i = 0; i < dir_sectors; i++)
    {
        pflex_directory[i] = { };

        if (i < dir_sectors - 1)
        {
            pflex_directory[i].next_trk =
                static_cast<Byte>((i + 5) / MAX_SECTOR);
            pflex_directory[i].next_sec =
                static_cast<Byte>(((i + 5) % MAX_SECTOR) + 1);
        }
    }
} // initialize_flex_directory


std::string NafsDirectoryContainer::get_unix_filename(const char *pfn) const
{
    if (*pfn != DE_EMPTY && *pfn != DE_DELETED)
    {
        std::string::size_type length;

        length = std::min<size_t>(strlen(pfn), FLEX_BASEFILENAME_LENGTH);
        std::string name(pfn, length);
        length = std::min<size_t>(strlen(pfn + FLEX_BASEFILENAME_LENGTH),
                          FLEX_FILEEXT_LENGTH);
        std::string ext(pfn + FLEX_BASEFILENAME_LENGTH, length);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return name + '.' + ext;
    }

    return std::string();
} // get_unix_filename

// return unix filename from a given file_id
std::string NafsDirectoryContainer::unix_filename(SWord file_id) const
{
    if (file_id < 0)
    {
        std::stringstream filename;

        filename << "tmp" << std::setw(2) << std::setfill('0')
                 << NEW_FILE1 - file_id;
        return filename.str();
    }
    else
    {
        const char *filename =
            (const char *)
            &pflex_directory[file_id / 10].dir_entry[file_id % 10].filename;
        return get_unix_filename(filename);
    }
} // unix_filename

// return the record nr (beginning from 0) of a new file which first
// sector has index 'index' into flex link table
Word NafsDirectoryContainer::record_nr_of_new_file(SWord new_file_index,
        Word index) const
{
    SWord   i;
    Word    record_nr;

    record_nr = 0;
    i = pnew_file[new_file_index].first.st.trk * MAX_SECTOR +
        pnew_file[new_file_index].first.st.sec - 1;

    while (i != index && i >= 0)
    {
        record_nr++;
        i = pflex_links[i].next.st.trk * MAX_SECTOR +
            pflex_links[i].next.st.sec - 1;
    } // while

    return record_nr;
}

SWord NafsDirectoryContainer::index_of_new_file(Byte track, Byte sector)
{
    SWord           i;
    char            path[PATH_MAX + 1];
    s_new_file      *pnf;

    i = 0;

    while (pnew_file[i].first.sec_trk && i < new_files)
    {
        if (track == pnew_file[i].next.st.trk &&
            sector == pnew_file[i].next.st.sec)
        {
            return i;
        }

        i++;
    }

    if (i >= new_files)
    {
        // new file table is full, increase new file table by 2
        pnf = new(std::nothrow) s_new_file[new_files + 2];

        if (pnf == nullptr)
        {
            return -1;    // can't allocate memory
        }

        memcpy(pnf, &pnew_file[0], new_files * sizeof(s_new_file));
        pnew_file.reset(pnf);
        new_files += 2;
        pnew_file[new_files - 2].first.sec_trk = 0;
        pnew_file[new_files - 1].first.sec_trk = 0;
    }

    strcpy(pnew_file[i].filename, unix_filename(NEW_FILE1 - i).c_str());
    strcpy(path, directory.c_str());
    strcat(path, PATHSEPARATORSTRING);
    strcat(path, &pnew_file[i].filename[0]);
    pnew_file[i].fp = fopen(path, "wb+");

    if (pnew_file[i].fp == nullptr)
    {
        return -1;
    }

    pnew_file[i].first.st.trk = track;
    pnew_file[i].first.st.sec = sector;
    pnew_file[i].next.sec_trk = 0;
    pnew_file[i].f_record = 0;

    return i;
} // index_of_new_file


bool NafsDirectoryContainer::extend_directory(SWord index,
    const s_dir_sector &dir_sector)
{
    // increase flex_directory by one sector
    s_dir_sector *pfd = new(std::nothrow) s_dir_sector[dir_sectors + 1];

    if (pfd == nullptr)
    {
        return false;    // can't allocate memory
    }

    memcpy(pfd, &pflex_directory[0], dir_sectors * sizeof(s_dir_sector));
    memcpy(pfd + dir_sectors, &dir_sector, sizeof(s_dir_sector));
    pflex_links[index].f_record = dir_sectors;
    pflex_links[index].file_id  = DIRECTORY;
    pflex_directory.reset(pfd);
    dir_sectors++;
    dir_extend.sec_trk = 0;// reset directory extend track/sector
    return true;
} // extend_directory


st_t NafsDirectoryContainer::link_address() const
{
    st_t link;
    s_dir_entry *pd;
    Word i;

    link.sec_trk = 0;

    for (i = 0; i < dir_sectors * 10; i++)
    {
        pd = &pflex_directory[i / 10].dir_entry[i % 10];

        if (!strncmp(pd->filename, "FLEX\0\0\0\0", 8) &&
            !strncmp(pd->file_ext, "SYS", 3))
        {
            link.st.trk = pd->start_trk;
            link.st.sec = pd->start_sec;
            break;
        }
    } // for

    return link;
}  // link_address


//if directory can't be extended return -1
SWord NafsDirectoryContainer::next_free_dir_entry()
{
    SWord i;
    Word index;
    s_sys_info_sector *psis;
    s_dir_sector dir_sector;
    Word trk, sec;

    for (i = 0; i < dir_sectors * 10; i++)
    {
        if (pflex_directory[i / 10].dir_entry[i % 10].filename[0] == DE_EMPTY)
        {
            return i;
        }
    } // for

    psis = &pflex_sys_info[0];

    memset(reinterpret_cast<char *>(&dir_sector), 0, sizeof(dir_sector));

    Word record_nr = dir_sectors - INIT_DIR_SECTORS + 1;
    dir_sector.record_nr[0] = static_cast<Byte>(record_nr >> 8);
    dir_sector.record_nr[1] = static_cast<Byte>(record_nr & 0xFF);
    trk = psis->fc_start_trk;
    sec = psis->fc_start_sec;
    index = trk * MAX_SECTOR + sec - 1;

    if (extend_directory(index, dir_sector))
    {
        pflex_directory[dir_sectors - 2].next_trk = (Byte)trk;
        pflex_directory[dir_sectors - 2].next_sec = (Byte)sec;

        if (++psis->fc_start_sec > MAX_SECTOR)
        {
            psis->fc_start_sec = 1;
            psis->fc_start_trk++;
        }

        if (--psis->free[1] == 0xff)
        {
            psis->free[0]--;
        }

        return i;
    }
    else
    {
        return -1;
    }
}  // next_free_dir_entry


void NafsDirectoryContainer::initialize_flex_link_table()
{
    Word i;
    Word fc_start = MAX_SECTOR;
    s_link_table *plink;

    // entries for system and directory sectors
    for (i = 0; i < fc_start; i++)
    {
        plink = &pflex_links[i];
        plink->next.sec_trk = 0;
        plink->record_nr[0] = 0;
        plink->record_nr[1] = 0;
        plink->f_record = i < 4 ? 0 : i - 4;
        plink->file_id = i < 4 ? SYSTEM : DIRECTORY;
    } // for

    for (i = fc_start; i < LINK_TABLE_SIZE; i++)
    {
        plink = &pflex_links[i];

        if (i == LINK_TABLE_SIZE - 1)
        {
            plink->next.sec_trk = 0;
        }
        else
        {
            plink->next.st.trk = static_cast<Byte>((i + 1) / MAX_SECTOR);
            plink->next.st.sec = static_cast<Byte>(((i + 1) % MAX_SECTOR) + 1);
        }

        plink->record_nr[0] = 0;
        plink->record_nr[1] = 0;
        plink->f_record = 0;
        plink->file_id = FREE_CHAIN;
    }

    Word free = LINK_TABLE_SIZE - fc_start;

    // and now update system info sectors
    for (i = 0; i < 2; i++)
    {
        s_sys_info_sector *psis = &pflex_sys_info[i];

        psis->fc_start_trk = static_cast<Byte>(fc_start / MAX_SECTOR);
        psis->fc_start_sec = static_cast<Byte>((fc_start % MAX_SECTOR) + 1);
        psis->fc_end_trk = MAX_TRACK;
        psis->fc_end_sec = MAX_SECTOR;
        psis->free[0] = free >> 8;
        psis->free[1] = free & 0xff;
    }
} // initialize_flex_link_table

void NafsDirectoryContainer::initialize_new_file_table()
{
    Word i;

    for (i = 0; i < new_files; i++)
    {
        pnew_file[i].first.sec_trk = 0;
        pnew_file[i].next.sec_trk = 0;
    }
} // initialize_new_file_table

// check for any open file
bool NafsDirectoryContainer::open_files()
{
    for (Word i = 0; i < new_files; i++)
    {
        if (pnew_file[i].first.sec_trk != 0)
        {
            return true;
        }
    }

    return false;
} // open_files


void NafsDirectoryContainer::close_new_files()
{
    Word i;
    Byte first = 1;
    std::string msg;

    for (i = 0; i < new_files; i++)
    {
        if (pnew_file[i].first.sec_trk != 0)
        {
            if (first)
            {
                msg = "There are still open files\n"
                       "temporarily stored as:\n";
                first = 0;
            }

            fclose(pnew_file[i].fp);

            msg += "   ";
            msg += pnew_file[i].filename;
            msg += "\n";
        }
    }

    if (!msg.empty())
    {
#ifdef _WIN32
    std::string title(PROGRAMNAME " warning");
#ifdef UNICODE
    MessageBox(nullptr, ConvertToUtf16String(msg).c_str(),
        ConvertToUtf16String(title).c_str(),
        MB_OK | MB_ICONEXCLAMATION);
#else
    MessageBox(nullptr, msg.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);
#endif

#endif
#ifdef UNIX
    fprintf(stderr, "%s", msg.c_str());
#endif
    }
} // close_new_files


void NafsDirectoryContainer::free_memory()
{
    pflex_links.reset(nullptr);
    pflex_directory.reset(nullptr);
    pflex_sys_info.reset(nullptr);
    pflex_unused.reset(nullptr);
    pnew_file.reset(nullptr);
} // free_memory

// if file won't fit return 0 otherwise return 1
bool NafsDirectoryContainer::add_to_link_table(
    SWord dir_index,
    off_t size,
    Byte random,
    st_t &begin,
    st_t &end)
{
    off_t i, free, sector_begin, records;
    s_link_table *plink;
    s_sys_info_sector *psis = &pflex_sys_info[0];

    free = (psis->free[0] << 8) + psis->free[1];

    if (size > static_cast<off_t>(free * 252L))
    {
        return false;
    }

    records = (size + 251L) / 252;
    begin.st.sec = psis->fc_start_sec;
    begin.st.trk = psis->fc_start_trk;
    sector_begin = (begin.st.trk * MAX_SECTOR) + begin.st.sec - 1;

    for (i = 1; i <= records; i++)
    {
        plink = &pflex_links[i + sector_begin - 1];

        if (i == records)
        {
            plink->next.sec_trk = 0;
        }

        if (random)
        {
            plink->record_nr[0] = static_cast<Byte>(i > 2 ? (i - 2) >> 8 : 0);
            plink->record_nr[1] = i > 2 ? (i - 2) & 0xff : 0;
        }
        else
        {
            plink->record_nr[0] = static_cast<Byte>(i >> 8);
            plink->record_nr[1] = i & 0xff;
        }

        plink->f_record = static_cast<Word>(i - 1);
        plink->file_id = dir_index;
    }

    end.st.sec = ((i + sector_begin - 2) % MAX_SECTOR) + 1;
    end.st.trk = static_cast<Byte>((i + sector_begin - 2) / MAX_SECTOR);
    // update sys info sector
    psis->fc_start_sec = ((i + sector_begin - 1) % MAX_SECTOR) + 1;
    psis->fc_start_trk = static_cast<Byte>((i + sector_begin - 1) / MAX_SECTOR);
    psis->free[0] = static_cast<Byte>((free - records) >> 8);
    psis->free[1] = (free - records) & 0xff;

    return true;
} // add_to_link_table


void NafsDirectoryContainer::add_to_directory(
    char *name,
    char *ext,
    SWord dir_index,
    Byte random,
    const struct stat &stat,
    const st_t &begin,
    const st_t &end,
    Byte wp)
{
    s_dir_entry *pd;
    struct tm *lt;
    SWord records;

    lt = localtime(&(stat.st_mtime));
    records = static_cast<SWord>((stat.st_size + 251L) / 252);
    pd = &pflex_directory[dir_index / 10].dir_entry[dir_index % 10];
    memset(pd->filename, 0, FLEX_BASEFILENAME_LENGTH);
    strncpy(pd->filename, name, FLEX_BASEFILENAME_LENGTH);
    memset(pd->file_ext, 0, FLEX_FILEEXT_LENGTH);
    strncpy(pd->file_ext, ext, FLEX_FILEEXT_LENGTH);
    pd->file_attr   = wp ? 0xc0 : 0x00;
    pd->start_trk   = begin.st.trk;
    pd->start_sec   = begin.st.sec;
    pd->end_trk     = end.st.trk;
    pd->end_sec     = end.st.sec;
    pd->records[0]  = records >> 8;
    pd->records[1]  = records & 0xff;
    pd->sector_map  = random ? 0x02 : 0x00;
    pd->month = static_cast<Byte>(lt->tm_mon + 1);
    pd->day = static_cast<Byte>(lt->tm_mday);
    pd->year = static_cast<Byte>(lt->tm_year);
} // add_to_directory

bool NafsDirectoryContainer::is_in_file_random(const char *ppath,
        const char *pfilename)
{
    char str[PATH_MAX + 1];

    strcpy(str, ppath);
    strcat(str, PATHSEPARATORSTRING RANDOM_FILE_LIST);

    BFilePtr fp(str, "r");

    if (fp != nullptr)
    {
        while (!feof((FILE *)fp) && fgets(str, PATH_MAX, fp) != nullptr)
        {
            if (strchr(str, '\n'))
            {
                *strchr(str, '\n') = '\0';
            }

            if (strcmp(pfilename, str) == 0)
            {
                return true;
            }
        }
    } // if

    return false;

} // is_in_file_random

void NafsDirectoryContainer::modify_random_file(char *path,
        const struct stat &stat, const st_t &begin)
{
    Byte file_sector_map[252 * 2];
    SDWord data_size;
    Word i, n, index;

    data_size = stat.st_size - (252L * 2);

    if (data_size >= 252L * 2)
    {
        index = (begin.st.trk * MAX_SECTOR + begin.st.sec) - 1 + 2;

        for (i = 0; i < 252 * 2; i++)
        {
            file_sector_map[i] = 0;
        }

        n = 0;

        for (n = 0; n < (data_size / (252L * 255)) ; n++)
        {
            file_sector_map[3 * n] = static_cast<Byte>(index / MAX_SECTOR);
            file_sector_map[3 * n + 1] =
                static_cast<Byte>((index % MAX_SECTOR) + 1);
            file_sector_map[3 * n + 2] = 255;
            index += 255;
        } // for

        i = (Word)(data_size % (252L * 255));

        if (i != 0)
        {
            file_sector_map[3 * n] = static_cast<Byte>(index / MAX_SECTOR);
            file_sector_map[3 * n + 1] =
                static_cast<Byte>((index % MAX_SECTOR) + 1);
            file_sector_map[3 * n + 2] = static_cast<Byte>((i + 251) / 252);
        } // if

        BFilePtr fp(path, "rb+");

        if (fp != nullptr)
        {
            fwrite(file_sector_map, 252, 2, fp);
        }
    } // if
} // modify_random_file

// dwp = write protect for drive

void NafsDirectoryContainer::fill_flex_directory(Byte dwp)
{
#ifdef _WIN32
    HANDLE hdl;
    WIN32_FIND_DATA pentry;
#endif
#ifdef UNIX
    DIR *pd;
    struct dirent *pentry;
#endif
    char name[9], ext[4], path[PATH_MAX + 1];
    std::string filename;
    SWord dir_index = 0;
    Word random;
    st_t begin, end;
    struct stat sbuf;

    initialize_flex_directory();
    initialize_flex_link_table();
#ifdef _WIN32
    strcpy(path, directory.c_str());
    strcat(path, PATHSEPARATORSTRING "*.*");

#ifdef UNICODE
    hdl = FindFirstFile(ConvertToUtf16String(path).c_str(), &pentry);
#else
    hdl = FindFirstFile(path, &pentry);
#endif
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
#endif
#ifdef UNIX

            if ((pd = opendir(directory.c_str())) != nullptr)
            {
                while ((pentry = readdir(pd)) != nullptr)
                {
#endif
#ifdef _WIN32
#ifdef UNICODE
                    filename = ConvertToUtf8String(pentry.cFileName);
#else
                    filename = pentry.cFileName;
#endif
                    std::transform(filename.begin(), filename.end(),
                        filename.begin(), ::tolower);
#endif
#ifdef UNIX
                    filename = pentry->d_name;
#endif
                    random = 0;
                    strcpy(path, directory.c_str());
                    strcat(path, PATHSEPARATORSTRING);
                    strcat(path, filename.c_str());

                    if (IsFlexFilename(filename.c_str(), name, ext) &&
                        !stat(path, &sbuf) && (S_ISREG(sbuf.st_mode)) &&
                        // exclude file "random":
                        strcmp(filename.c_str(), RANDOM_FILE_LIST) &&
                        sbuf.st_size > 0 &&
                        (dir_index = next_free_dir_entry()) >= 0)
                    {
#ifdef _WIN32
                        random = (pentry.dwFileAttributes &
                                  FILE_ATTRIBUTE_HIDDEN) ? 1 : 0;
#endif
#ifdef UNIX
                        random = sbuf.st_mode & S_IXUSR;
#endif

                        // CDFS-Support: look for file name in file 'random'
                        if (dwp)
                        {
                            random = is_in_file_random(directory.c_str(),
                                                       filename.c_str());
                        }

                        if (add_to_link_table(dir_index, sbuf.st_size,
                                              (Byte)random, begin, end))
                        {
                            add_to_directory(name, ext, dir_index,
                                             (Byte)random, sbuf, begin, end,
                                             dwp || access(path, W_OK));

                            // Unfinished: don't write sector map if write
                            // protected
                            if (random && !dwp)
                            {
                                modify_random_file(path, sbuf, begin);
                            }
                        }
                    }

#ifdef _WIN32
                }

                while (FindNextFile(hdl, &pentry) != 0);

                FindClose(hdl);
#endif
#ifdef UNIX
            } // while

            closedir(pd);
#endif
    } //if
} // fill_flex_directory


void NafsDirectoryContainer::initialize_flex_sys_info_sectors(Word number)
{
    Word i;
    char name[9], ext[4];
    const char *pName;
    struct stat sbuf;
    struct tm *lt;
    s_sys_info_sector *psis;

    if (!stat(directory.c_str(), &sbuf))
    {
        psis = &pflex_sys_info[0];
        lt = localtime(&(sbuf.st_mtime));
        name[0] = '\0';
        ext[0]  = '\0';
        pName = directory.c_str() + directory.length() - 1;

        if (*pName == PATHSEPARATOR)
        {
            pName--;
        }

        while (pName != directory.c_str() && *pName != PATHSEPARATOR)
        {
            pName--;
        }

        if (*pName == PATHSEPARATOR)
        {
            pName++;
        }

        if (sscanf(pName, "%8[a-zA-Z0-9_-]", name) != 1)
        {
            strcpy(name, "UNIX");
        }
        else
        {
            strupper(name);
            strupper(ext);
        } // else

        for (i = 0; i < 16; i++)
        {
            psis->unused1[i] = 0;
        }

        strncpy(psis->disk_name, name, 8);
        strncpy(psis->disk_ext, ext, 3);
        psis->disk_number[0] = number >> 8;
        psis->disk_number[1] = number & 0xff;
        psis->fc_start_trk = 0;
        psis->fc_start_sec = 0;
        psis->fc_end_trk = 0;
        psis->fc_end_sec = 0;
        psis->free[0] = 0;
        psis->free[1] = 0;
        psis->month = static_cast<Byte>(lt->tm_mon + 1);
        psis->day = static_cast<Byte>(lt->tm_mday);
        psis->year = static_cast<Byte>(lt->tm_year);
        psis->last_trk = MAX_TRACK;
        psis->last_sec = MAX_SECTOR;

        for (i = 0; i < 216; i++)
        {
            psis->unused2[i] = 0;
        }

        pflex_sys_info[1] = pflex_sys_info[0];
    } // if
} // initialize_flex_sys_info_sectors


// I don't know what this sector should be used for but
// emulate it anyway
void NafsDirectoryContainer::initialize_flex_unused_sector()
{
    Word i;

    pflex_unused->next_trk = 0x00;
    pflex_unused->next_sec = 0x03;

    for (i = 0; i < 254; i++)
    {
        pflex_unused->unused[i] = 0;
    }
} // initialize_flex_unused_sector


// change the file id in flex_links
void NafsDirectoryContainer::change_file_id(SWord index, SWord old_id,
        SWord new_id) const
{
    SWord i = index;

    while (i >= 0 && pflex_links[i].file_id == old_id)
    {
        pflex_links[i].file_id = new_id;
        i = pflex_links[i].next.st.trk * MAX_SECTOR +
            pflex_links[i].next.st.sec - 1;
    } // while
} // change_file_id


void NafsDirectoryContainer::check_for_delete(SWord dir_index,
        const s_dir_sector &dir_sector) const
{
    Word i;
    SWord index;
    char path[PATH_MAX + 1];
    const char *pfilename;
    s_dir_sector *pd;

    pd = &pflex_directory[dir_index];

    for (i = 0; i < 10; i++)
    {
        if (dir_sector.dir_entry[i].filename[0] == DE_DELETED &&
            pd->dir_entry[i].filename[0] != DE_DELETED)
        {
            pfilename = unix_filename(dir_index * 10 + i).c_str();
            index = pd->dir_entry[i].start_trk * MAX_SECTOR +
                    pd->dir_entry[i].start_sec - 1;
            strcpy(path, directory.c_str());
            strcat(path, PATHSEPARATORSTRING);
            strcat(path, pfilename);
            unlink(path);
            change_file_id(index, dir_index * 10 + i, FREE_CHAIN);
#ifdef DEBUG_FILE
            LOG_X("     delete %s\n", pfilename);
#endif
            break;
        } // if
    } // for

} // check_for_delete


void NafsDirectoryContainer::check_for_rename(SWord dir_index,
        const s_dir_sector &dir_sector) const
{
    Word i;
    char old_path[PATH_MAX + 1], new_path[PATH_MAX + 1];
    std::string filename1, filename2;
    const char *pfilename1, *pfilename2;

    for (i = 0; i < 10; i++)
    {
        filename1 = unix_filename(dir_index * 10 + i);
        filename2 = get_unix_filename(dir_sector.dir_entry[i].filename);

        pfilename1 = filename1.c_str();
        pfilename2 = filename2.c_str();

        if (*pfilename1 && *pfilename2 &&
            strcmp(pfilename1, pfilename2) != 0)
        {
            strcpy(old_path, directory.c_str());
            strcat(old_path, PATHSEPARATORSTRING);
            strcat(old_path, pfilename1);
            strcpy(new_path, directory.c_str());
            strcat(new_path, PATHSEPARATORSTRING);
            strcat(new_path, pfilename2);
            rename(old_path, new_path);
#ifdef DEBUG_FILE
            LOG_XX("     rename %s to %s\n",
                    pfilename1, pfilename2);
#endif
            break;
        } // if
    } // for
} // check_for_rename


void NafsDirectoryContainer::check_for_extend(SWord dir_index,
        const s_dir_sector &dir_sector)
{
    s_dir_sector *pd;

    pd = &pflex_directory[dir_index];

    if (!pd->next_sec && !pd->next_trk &&
        (dir_sector.next_trk || dir_sector.next_sec))
    {
        dir_extend.st.trk = dir_sector.next_trk;
        dir_extend.st.sec = dir_sector.next_sec;
    } // if
} // check_for_extend


// return -1 if not successful otherwise return 0
// years < 75 will be represented as >= 2000
SWord NafsDirectoryContainer::set_file_time(char *ppath, Byte month,
        Byte day, Byte year) const
{
    struct stat    statbuf;
    struct utimbuf timebuf;
    struct tm      file_time;

    if (stat(ppath, &statbuf) >= 0)
    {
        timebuf.actime = statbuf.st_atime;
        file_time.tm_sec = 0;
        file_time.tm_min = 0;
        file_time.tm_hour = 12;
        file_time.tm_mon = month - 1;
        file_time.tm_mday = day;
        file_time.tm_year = year < 75 ? year + 100 : year;
        file_time.tm_isdst = 0;
        timebuf.modtime = mktime(&file_time);

        if (timebuf.modtime >= 0 && utime(ppath, &timebuf) >= 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    } // if

    return -1;
} // set_file_time


void NafsDirectoryContainer::check_for_new_file(SWord dir_index,
        const s_dir_sector &dir_sector) const
{
    Word i, j;
    SWord index;
    char old_path[PATH_MAX + 1], new_path[PATH_MAX + 1];
#ifdef UNIX
    struct stat sbuf;
#endif

    j = 0;

    while (pnew_file[j].first.sec_trk && j < new_files)
    {
        for (i = 0; i < 10; i++)
        {
            if (
               pnew_file[j].first.st.sec == dir_sector.dir_entry[i].start_sec &&
               pnew_file[j].first.st.trk == dir_sector.dir_entry[i].start_trk)
            {
                index = pnew_file[j].first.st.trk * MAX_SECTOR +
                        pnew_file[j].first.st.sec - 1;
                change_file_id(index, NEW_FILE1 - j,
                                10 * dir_index + i);
                fclose(pnew_file[j].fp);
                strcpy(old_path, directory.c_str());
                strcat(old_path, PATHSEPARATORSTRING);
                strcat(old_path, pnew_file[j].filename);

                // check for random file, if true set user execute bit
                if (dir_sector.dir_entry[i].sector_map & 0x02)
#ifdef _WIN32
#ifdef UNICODE
                SetFileAttributes(ConvertToUtf16String(old_path).c_str(),
                    FILE_ATTRIBUTE_HIDDEN);
#else
                SetFileAttributes(old_path, FILE_ATTRIBUTE_HIDDEN);
#endif

#endif
#ifdef UNIX

                if (!stat(old_path, &sbuf))
                {
                    chmod(old_path, sbuf.st_mode | S_IXUSR);
                }

#endif
                std::string new_name = unix_filename(
                            pflex_links[index].file_id);
                strcpy(new_path, directory.c_str());
                strcat(new_path, PATHSEPARATORSTRING);
                strcat(new_path, new_name.c_str());
                rename(old_path, new_path);
#ifdef DEBUG_FILE
                LOG_XX("     new file %s, was %s\n",
                        unix_filename(pflex_links[index].file_id),
                        pnew_file[j].filename);
#endif
                set_file_time(new_path,
                    dir_sector.dir_entry[i].month,
                    dir_sector.dir_entry[i].day,
                    dir_sector.dir_entry[i].year);

                // remove entry in new file table and
                // move following entries
                while (pnew_file[j + 1].first.sec_trk &&
                        j < new_files - 1)
                {
                    strcpy(pnew_file[j].filename,
                            &pnew_file[j + 1].filename[0]);
                    pnew_file[j].first.sec_trk =
                        pnew_file[j + 1].first.sec_trk;
                    pnew_file[j].next.sec_trk =
                        pnew_file[j + 1].next.sec_trk;
                    pnew_file[j].f_record = pnew_file[j + 1].f_record;
                    pnew_file[j].fp = pnew_file[j + 1].fp;
                    index = pnew_file[j].first.st.trk * MAX_SECTOR +
                            pnew_file[j].first.st.sec - 1;
                    change_file_id(index, NEW_FILE1 - j - 1, NEW_FILE1 - j);
                    j++;
                } // while

                pnew_file[j].first.sec_trk = 0;
            } // if
        } // for

        j++;
    } // while
} // check_for_new_file


Byte NafsDirectoryContainer::last_of_free_chain(Byte trk, Byte sec) const
{
    return (pflex_sys_info[0].fc_end_trk == trk &&
            pflex_sys_info[0].fc_end_sec == sec);
} // last_of_free_chain


// return true on success
bool NafsDirectoryContainer::ReadSector(Byte * buffer, int trk,
                                        int sec) const
{
    char path[PATH_MAX + 1];
    int index = trk * MAX_SECTOR + (sec - 1);
    st_t link;
    bool result = true;

#ifdef DEBUG_FILE
    LOG_XX("read: %02X/%02X ", trk, sec);
#endif
    s_link_table *pfl = &pflex_links[index];

    switch (pfl->file_id)
    {
        case SYSTEM :
#ifdef DEBUG_FILE
            LOG("systemsector\n");
#endif
            if (sec >= 3)
            {
                char *p = reinterpret_cast<char *>(&pflex_sys_info[sec-3]);
                memcpy(buffer, p, param.byte_p_sector);
                break;
            }
            else if (sec == 2)
            {
                char *p = reinterpret_cast<char *>(pflex_unused.get());
                memcpy(buffer, p, param.byte_p_sector);
                break;
            }
            else if (sec == 1)
            {
                FILE *fp;

                memset(buffer, 0, SECTOR_SIZE);

                link = link_address();
                strcpy(path, directory.c_str());
                strcat(path, PATHSEPARATORSTRING "boot");

                if ((fp = fopen(path, "rb")) != nullptr)
                {
                    size_t bytes = fread(buffer, 1, SECTOR_SIZE, fp);
                    if (bytes == SECTOR_SIZE)
                    {
                        buffer[3] = link.st.trk;
                        buffer[4] = link.st.sec;
                    }
                    else
                    {
                        // Default buffer content if no boot sector present.
                        // Jump to monitor program warm start entry point.
                        buffer[0] = 0x7E; // JMP $F02D
                        buffer[1] = 0xF0;
                        buffer[2] = 0x2D;
                    }
                    fclose(fp);
                }

                break;
            }

            memset(buffer, 0, SECTOR_SIZE);
            break;

        case DIRECTORY:
#ifdef DEBUG_FILE
            LOG("directorysector\n");
#endif
            {
                Word di = pfl->f_record;
                char *p = reinterpret_cast<char *>(&pflex_directory[di]);
                memcpy(buffer, p, param.byte_p_sector);
            }
            break;

        case FREE_CHAIN :
#ifdef DEBUG_FILE
            LOG("free chain\n");
#endif
            buffer[0] = pfl->next.st.trk;
            buffer[1] = pfl->next.st.sec;
            buffer[2] = pfl->record_nr[0];
            buffer[3] = pfl->record_nr[1];

            // free chain sector reads always
            // filled with zero
            memset(buffer + 4, 0, SECTOR_SIZE - 4);

            break;

        default :
            if (pfl->file_id >= 0)
            {
                // Read from an existing file.
                FILE *fp;

#ifdef DEBUG_FILE
                LOG_X("sector of file %s\n", unix_filename(pfl->file_id));
#endif
                strcpy(path, directory.c_str());
                strcat(path, PATHSEPARATORSTRING);
                strcat(path, unix_filename(pfl->file_id).c_str());

                if ((fp = fopen(path, "rb")) != nullptr &&
                    !fseek(fp, (long)(pfl->f_record * 252L),
                            SEEK_SET))
                {
                    size_t bytes = fread(buffer + 4, 1, 252, fp);
                    fclose(fp);

                    // stuff last sector of file with 0
                    if (bytes < 252)
                    {
                        memset(buffer + 4 + bytes, 0,
                                SECTOR_SIZE - 4 - bytes);
                    }
                }
                else     // unable to read sector
                {
                    result = false;
                } // else
            }
            else
            {
                // new file with name tmpXX
                FILE *fp = pnew_file[NEW_FILE1 - pfl->file_id].fp;

                if (!fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET))
                {
                    size_t bytes = fread(buffer + 4, 1, 252, fp);

                    // stuff last sector of file with 0
                    if (bytes < 252)
                    {
                        memset(buffer + 4 + bytes, 0,
                                SECTOR_SIZE - 4 - bytes);
                    }

                    fseek(fp, 0L, SEEK_END); // position end of file
                }
                else     // unable to read sector
                {
                    result = false;
                } // else
            } // else

            buffer[0] = pfl->next.st.trk;
            buffer[1] = pfl->next.st.sec;
            buffer[2] = pfl->record_nr[0];
            buffer[3] = pfl->record_nr[1];
            break;
    } // switch

    return result;
} //ReadSector


bool NafsDirectoryContainer::WriteSector(const Byte * buffer, int trk,
        int sec)
{
    char path[PATH_MAX + 1];
    SWord new_file_index;
    bool result = true;
    Byte track = static_cast<Byte>(trk);
    Byte sector = static_cast<Byte>(sec);
    SWord index = track * MAX_SECTOR + (sector - 1);
    s_link_table *pfl = &pflex_links[index];

#ifdef DEBUG_FILE
    LOG_XX("write: %02X/%02X ", trk, sec);
#endif

    switch (pfl->file_id)
    {
        case SYSTEM :
#ifdef DEBUG_FILE
            LOG("systemsector\n");
#endif
            if (sector >= 3)
            {
                char *p;

                p = reinterpret_cast<char *>(&pflex_sys_info[sector - 3]);
                memcpy(p, buffer, param.byte_p_sector);
            }
            else if (sector == 2)
            {
                char *p = reinterpret_cast<char *>(pflex_unused.get());
                memcpy(p, buffer, param.byte_p_sector);
            }
            else if (sector == 1)
            {
                FILE *fp;

                strcpy(path, directory.c_str());
                strcat(path, PATHSEPARATORSTRING "boot");

                if ((fp = fopen(path, "wb")) != nullptr)
                {
                    size_t bytes = fwrite(buffer, 1, SECTOR_SIZE, fp);
                    if (bytes != SECTOR_SIZE)
                    {
                        result = false;
                    }
                    fclose(fp);
                }

                break;
            }

            break;

        case DIRECTORY:
#ifdef DEBUG_FILE
            LOG("directorysector\n");
#endif
            {
                Word di = pfl->f_record;
                s_dir_sector &dir_sector = (s_dir_sector &)buffer;
                char *p = reinterpret_cast<char *>(&pflex_directory[di]);
                check_for_delete(di, dir_sector);
                check_for_new_file(di, dir_sector);
                check_for_rename(di, dir_sector);
                check_for_extend(di, dir_sector);
                memcpy(p, buffer, param.byte_p_sector);
            }
            break;

        case FREE_CHAIN :
#ifdef DEBUG_FILE
            LOG("free chain\n");
#endif

            if (dir_extend.st.trk == track && dir_extend.st.sec == sector)
            {
                extend_directory(index, (const s_dir_sector &)buffer);
#ifdef DEBUG_FILE
                LOG("      extend directory\n");
#endif
                break;
            } // if

            if (last_of_free_chain(track, sector) && (buffer[1] || buffer[0]))
            {
                pfl->next.st.trk  = buffer[0];
                pfl->next.st.sec  = buffer[1];
                pfl->record_nr[0] = buffer[2];
                pfl->record_nr[1] = buffer[3];
#ifdef DEBUG_FILE
                LOG("      file deleted\n");
#endif
                break;
            }

            if ((new_file_index = index_of_new_file(track, sector)) < 0)
            {
#ifdef DEBUG_FILE
                LOG("   ** error: unable to create new file\n");
#endif
                result = false; // no more new files can be created
                break;
            }

#ifdef DEBUG_FILE
            LOG_X("      file %s\n", pnew_file[new_file_index].filename);
#endif
            {
                FILE *fp = pnew_file[new_file_index].fp;
                pnew_file[new_file_index].next.st.trk = buffer[0];
                pnew_file[new_file_index].next.st.sec = buffer[1];
                pfl->file_id  = NEW_FILE1 - new_file_index;
                //pfl->f_record = (pfs->pnew_file[new_file_index].f_record++;
                //pfl->f_record = ((sector_buffer[2] << 8) |
                // sector_buffer[3]) - 1;
                pfl->next.st.trk = buffer[0];
                pfl->next.st.sec = buffer[1];
                pfl->record_nr[0] = buffer[2];
                pfl->record_nr[1] = buffer[3];
                pfl->f_record = record_nr_of_new_file(new_file_index, index);

                if (ftell(fp) != (pfl->f_record * 252L) &&
                    fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET) != 0)
                {
                    result = false;
                }
                else if (fwrite(buffer + 4, 1, 252, fp) != 252)
                {
                    result = false;
                }

                // (pfs->pnew_file[new_file_index].f_record++;
            }
            break;

        default :
            if (pfl->file_id >=  0)
            {
                FILE *fp;
#ifdef DEBUG_FILE
                LOG_X("sector of file %s\n", unix_filename(pfl->file_id));
#endif
                strcpy(path, directory.c_str());
                strcat(path, PATHSEPARATORSTRING);
                strcat(path, unix_filename(pfl->file_id).c_str());
                pfl->next.st.trk = buffer[0];
                pfl->next.st.sec = buffer[1];
                pfl->record_nr[0] = buffer[2];
                pfl->record_nr[1] = buffer[3];

                if ((fp = fopen(path, "rb+")) == nullptr)
                {
                    result = false;
                }
                else
                {
                    if (ftell(fp) != pfl->f_record &&
                        fseek(fp, (long)(pfl->f_record * 252L),
                                SEEK_SET) != 0)
                    {
                        result = false;
                    }
                    else if (fwrite(buffer + 4, 1, 252, fp) != 252)
                    {
                        result = false;
                    }

                    fclose(fp);
                } // else
            }
            else
            {
#ifdef DEBUG_FILE
                LOG_X("sector of new file %s\n",
                        unix_filename(pfl->file_id));
#endif
                FILE *fp = pnew_file[NEW_FILE1 - pfl->file_id].fp;

                if (ftell(fp) != pfl->f_record &&
                    fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET) != 0)
                {
                    result = false;
                }
                else if (fwrite(buffer + 4, 1, 252, fp) != 252)
                {
                    result = false;
                }
            } //else

            pfl->next.st.trk = buffer[0];
            pfl->next.st.sec = buffer[1];
            pfl->record_nr[0] = buffer[2];
            pfl->record_nr[1] = buffer[3];
    } // switch

    return result;
} // WriteSector


void NafsDirectoryContainer::mount(Word number)
{
    Byte wp_flag;

    wp_flag = static_cast<Byte>(access(directory.c_str(), W_OK));
    initialize_header(wp_flag);
    initialize_new_file_table();
    initialize_flex_sys_info_sectors(number);
    initialize_flex_unused_sector();
    fill_flex_directory(wp_flag);
} // mount

#endif // #ifdef NAFS
