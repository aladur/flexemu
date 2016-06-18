/*
    ndircont.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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


#include <misc1.h>

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

// detailed debug messegas can be written to a debug file:
//#define DEBUG_FILE "debug.txt"
#ifdef DEBUG_FILE
    #include "debug.h"
#endif

int initializeWithZero(char *ptr, int count)
{
    char *p = ptr;

    for (int i = 0; i < count; i++)
    {
        p[i] = '\0';
    }

    return count;
}

NafsDirectoryContainer::NafsDirectoryContainer(const char *path) :
    dir(NULL), attributes(0)
{
    struct stat             sbuf;
    static Word             number = 0;

    if (path == NULL || stat(path, &sbuf) || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    dir = new std::string(path);

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

    if (pdir == NULL || stat(pdir, &sbuf) || !S_ISDIR(sbuf.st_mode))
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

bool NafsDirectoryContainer::IsContainerOpened(void) const
{
    return (dir != NULL);
}

std::string NafsDirectoryContainer::GetPath() const
{
    if (dir != NULL)
    {
        return *dir;
    }

    return "";
}

int NafsDirectoryContainer::Close(void)
{
    if (dir != NULL)
    {
        close_new_files();
        free_memory();
        delete dir;
        dir = NULL;
    }

    return 1;
}

bool NafsDirectoryContainer::GetInfo(FlexContainerInfo &info) const
{
    struct s_sys_info_sector buffer;

    if (dir == NULL)
    {
        return false;
    }

    if (!ReadSector((Byte *)&buffer, 0, 3))
    {
        throw FlexException(FERR_READING_TRKSEC, 0, 3, dir->c_str());
    }

    info.SetDate(buffer.day, buffer.month, buffer.year);
    info.SetTrackSector(buffer.last_trk + 1, buffer.last_sec);
    info.SetFree((((buffer.free[0] << 8) | buffer.free[1]) *
                  param.byte_p_sector) >> 10);
    info.SetTotalSize(((buffer.last_sec * (buffer.last_trk + 1)) *
                       param.byte_p_sector) >> 10);
    info.SetName(buffer.disk_name);
    info.SetPath(dir->c_str());
    info.SetType(param.type);
    info.SetAttributes(attributes);
    return true;
}

int NafsDirectoryContainer::GetBytesPerSector(void) const
{
    return param.byte_p_sector;
}

bool NafsDirectoryContainer::IsWriteProtected(void) const
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

int NafsDirectoryContainer::GetContainerType(void) const
{
    return param.type;
}

///////////////////////////////////////////////////////
// private interface
///////////////////////////////////////////////////////

void NafsDirectoryContainer::initialize_header(Byte wp)
{
    param.offset        = 0;
    param.write_protect = wp;
    param.max_sector    = MAX_SECTOR >> 1;
    param.max_sector0   = (INIT_DIR_SECTORS + 4) >> 1;
    param.max_track     = MAX_TRACK;
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector0 << 9;
    param.byte_p_track  = param.max_sector << 9;
    param.type                      = TYPE_DIRECTORY | TYPE_NAFS_DIRECTORY;
    pflex_links     = new struct s_link_table[LINK_TABLE_SIZE];
    initializeWithZero((char *)pflex_links, LINK_TABLE_SIZE *
                       sizeof(s_link_table));
    pflex_sys_info  = new struct s_sys_info_sector[2];
    initializeWithZero((char *)pflex_sys_info, 2 * sizeof(s_sys_info_sector));
    pflex_unused    = new struct s_unused_sector;
    initializeWithZero((char *)pflex_unused, sizeof(s_unused_sector));
    pflex_directory = new struct s_dir_sector[INIT_DIR_SECTORS];
    initializeWithZero((char *)pflex_directory,
                       INIT_DIR_SECTORS * sizeof(s_dir_sector));
    dir_sectors      = INIT_DIR_SECTORS;
    pnew_file       = new struct s_new_file[INIT_NEW_FILES];
    initializeWithZero((char *)pnew_file, INIT_NEW_FILES * sizeof(s_new_file));
    new_files        = INIT_NEW_FILES;
    dir_extend.sec_trk = 0;

    if (pflex_links == NULL || pflex_sys_info == NULL ||
        pflex_unused == NULL || pflex_directory == NULL ||
        pnew_file == NULL)
    {
        const char *pmsg = "Not enough memory, can't continue\n";

#ifdef WIN32
        MessageBox(NULL, pmsg, PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
#endif
#ifdef UNIX
        fprintf(stderr, "%s", pmsg);
#endif
        exit(EXIT_FAILURE);
    } // if
} // initialize_header

// if valid flex filename return is > 0 otherwise = 0
/*
    some examples:

    allowed:        x.a xx.a xxxxxxxx.a x xx xxxxxxxx
    not allowed:    x. .a xx. xxxxxxxxx.a X.a xxxxxxxxX.a
*/
bool NafsDirectoryContainer::IsFlexFilename(const char *pfilename,
        char *pname /* = NULL */,
        char *pext  /* = NULL */) const
{
    int result; // result from sscanf should be int
    char dot;
    char name[9];
    char ext[4];
    const char *format;

    dot    = '\0';
    format = "%1[A-Za-z]%7[A-Za-z0-9_-]";
    result = sscanf(pfilename, format, (char *)&name, (char *)&name + 1);

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

    result = sscanf(pfilename, format,
                    &dot, (char *)&ext, (char *)&ext + 1);

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
    Word i, j;
    char *p;

    for (i = 0; i < dir_sectors; i++)
    {
        p = (char *)(pflex_directory + i);

        for (j = 0; j < SECTOR_SIZE; j++)
        {
            *(p++) = 0x00;
        }

        if (i < dir_sectors - 1)
        {
            (pflex_directory + i)->next_trk = (i + 5) / MAX_SECTOR;
            (pflex_directory + i)->next_sec =
                ((i + 5) % MAX_SECTOR) + 1;
        } // if
    } // for
} // initialize_flex_directory


std::string NafsDirectoryContainer::get_unix_filename(const char *pfn) const
{
    if (*pfn != -1)
    {
        std::string name(pfn, 8);
        std::string ext(pfn + 8, 3);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return std::string(name + '.' + ext);
    } // if

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
        return filename.str().c_str();
    }
    else
    {
        const char *filename =
            (const char *)
            & ((pflex_directory +
                (file_id / 10))->dir_entry[file_id % 10].filename);
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
    i = (pnew_file + new_file_index)->first.st.trk * MAX_SECTOR +
        (pnew_file + new_file_index)->first.st.sec - 1;

    while (i != index && i >= 0)
    {
        record_nr++;
        i = (pflex_links + i)->next.st.trk * MAX_SECTOR +
            (pflex_links + i)->next.st.sec - 1;
    } // while

    return record_nr;
}

SWord NafsDirectoryContainer::index_of_new_file(Byte track, Byte sector)
{
    SWord           i;
    char            path[PATH_MAX + 1];
    s_new_file      *pnf;

    i = 0;

    while ((pnew_file + i)->first.sec_trk && i < new_files)
    {
        if (track == (pnew_file + i)->next.st.trk &&
            sector == (pnew_file + i)->next.st.sec)
        {
            return i;
        }

        i++;
    } // while

    if (i >= new_files)
    {
        // new file table is full, increase new file table by 2
        pnf = new(std::nothrow) struct s_new_file[new_files + 2];

        if (pnf == NULL)
        {
            return -1;    // can't allocate memory
        }

        memcpy(pnf, pnew_file, new_files * sizeof(struct s_new_file));
        delete [] pnew_file;
        pnew_file = pnf;
        new_files += 2;
        (pnew_file + (new_files - 2))->first.sec_trk = 0;
        (pnew_file + (new_files - 1))->first.sec_trk = 0;
    } // if

    strcpy((char *) & (pnew_file + i)->filename,
           unix_filename(NEW_FILE1 - i).c_str());
    strcpy((char *)&path, dir->c_str());
    strcat((char *)&path, PATHSEPARATORSTRING);
    strcat((char *)&path, (pnew_file + i)->filename);
    (pnew_file + i)->fp = fopen(path, "wb+");

    if ((pnew_file + i)->fp == NULL)
    {
        return -1;
    }

    (pnew_file + i)->first.st.trk = track;
    (pnew_file + i)->first.st.sec = sector;
    (pnew_file + i)->next.sec_trk = 0;
    (pnew_file + i)->f_record     = 0;
    return i;
} // index_of_new_file


Byte NafsDirectoryContainer::extend_directory(SWord index, s_dir_sector *pdb)
{
    struct s_dir_sector *pfd;

    // increase flex_directory by one sector
    pfd = new(std::nothrow) struct s_dir_sector[dir_sectors + 1];

    if (pfd == NULL)
    {
        return 0xff;    // can't allocate memory
    }

    memcpy(pfd, pflex_directory, dir_sectors * sizeof(struct s_dir_sector));
    memcpy(pfd + dir_sectors, pdb, SECTOR_SIZE);
    (pflex_links + index)->f_record = dir_sectors;
    (pflex_links + index)->file_id  = DIRECTORY;
    delete [] pflex_directory;
    pflex_directory = pfd;
    dir_sectors++;
    dir_extend.sec_trk = 0;// reset directory extend track/sector
    return 0;
} // extend_directory


t_st *NafsDirectoryContainer::link_address(void) const
{
    static t_st link;
    s_dir_entry *pd;
    Word i;

    link.sec_trk = 0;

    for (i = 0; i < dir_sectors * 10; i++)
    {
        pd = &((pflex_directory + (i / 10))->dir_entry[i % 10]);

        if (!strncmp(pd->filename, "FLEX\0\0\0\0", 8) &&
            !strncmp(pd->file_ext, "SYS", 3))
        {
            link.st.trk = pd->start_trk;
            link.st.sec = pd->start_sec;
            break;
        }
    } // for

    return &link;
}  // link_address


//if directory can't be extended return -1
SWord NafsDirectoryContainer::next_free_dir_entry()
{
    SWord i;
    Word j, index;
    s_sys_info_sector *psis;
    char sector_buffer[SECTOR_SIZE];
    Word trk, sec;

    for (i = 0; i < dir_sectors * 10; i++)
    {
        if (!((pflex_directory + (i / 10))->dir_entry[i % 10].filename[0]))
        {
            return i;
        }
    } // for

    psis = pflex_sys_info;

    for (j = 0; j < SECTOR_SIZE; j++)
    {
        sector_buffer[j] = 0x00;
    }

    sector_buffer[3] = dir_sectors - INIT_DIR_SECTORS + 1;
    trk = psis->fc_start_trk;
    sec = psis->fc_start_sec;
    index = trk * MAX_SECTOR + sec - 1;

    if (!extend_directory(index, (s_dir_sector *)sector_buffer))
    {
        (pflex_directory + dir_sectors - 2)->next_trk =
            (Byte)trk;
        (pflex_directory + dir_sectors - 2)->next_sec =
            (Byte)sec;

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
    Word i, fc_start, free;
    struct s_link_table *plink;
    struct s_sys_info_sector *psis;

    fc_start = MAX_SECTOR;

    // entries for system and directory sectors
    for (i = 0; i < fc_start; i++)
    {
        plink = pflex_links + i;
        plink->next.sec_trk = 0;
        plink->record_nr[0] = 0;
        plink->record_nr[1] = 0;
        plink->f_record = i < 4 ? 0 : i - 4;
        plink->file_id = i < 4 ? SYSTEM : DIRECTORY;
    } // for

    for (i = fc_start; i < LINK_TABLE_SIZE; i++)
    {
        plink = pflex_links + i;

        if (i == LINK_TABLE_SIZE - 1)
        {
            plink->next.sec_trk = 0;
        }
        else
        {
            plink->next.st.trk = (i + 1) / MAX_SECTOR;
            plink->next.st.sec = ((i + 1) % MAX_SECTOR) + 1;
        } // else

        plink->record_nr[0] = 0;
        plink->record_nr[1] = 0;
        plink->f_record = 0;
        plink->file_id = FREE_CHAIN;
    } // for

    free = LINK_TABLE_SIZE - fc_start;

    // and now update system info sectors
    for (i = 0; i < 2; i++)
    {
        psis = pflex_sys_info + i;
        psis->fc_start_trk = fc_start / MAX_SECTOR;
        psis->fc_start_sec = (fc_start % MAX_SECTOR) + 1;
        psis->fc_end_trk = MAX_TRACK;
        psis->fc_end_sec = MAX_SECTOR;
        psis->free[0] = free >> 8;
        psis->free[1] = free & 0xff;
    } // for
} // initialize_flex_link_table


void NafsDirectoryContainer::initialize_new_file_table()
{
    Word i;

    for (i = 0; i < new_files; i++)
    {
        (pnew_file + i)->first.sec_trk = 0;
        (pnew_file + i)->next.sec_trk = 0;
    } // for
} // initialize_new_file_table

// check for any open file
Byte NafsDirectoryContainer::open_files()
{
    for (Word i = 0; i < new_files; i++)
    {
        if ((pnew_file + i)->first.sec_trk != 0)
        {
            return 1;
        }
    } // for

    return 0;
} // open_files


void NafsDirectoryContainer::close_new_files()
{
    Word i;
    Byte first = 1;
    char msg[ERR_SIZE];

    msg[0] = '\0';

    for (i = 0; i < new_files; i++)
    {
        if ((pnew_file + i)->first.sec_trk != 0)
        {
            if (first)
            {
                strcpy(msg, "There are still open files\n"
                       "temporarily stored as:\n");
                first = 0;
            }

            fclose((pnew_file + i)->fp);

            if (strlen(msg) < (ERR_SIZE - 18))
            {
                strcat(msg, "   ");
                strcat(msg, (pnew_file + i)->filename);
                strcat(msg, "\n");
            } // if
        } // if
    } // for

    if (strlen(msg) > 0)
#ifdef WIN32
        MessageBox(NULL, msg, PROGRAMNAME " warning",
                   MB_OK | MB_ICONEXCLAMATION);

#endif
#ifdef UNIX
    fprintf(stderr, "%s", msg);
#endif
} // close_new_files


void NafsDirectoryContainer::free_memory()
{
    delete [] pflex_links;
    delete [] pflex_directory;
    delete [] pflex_sys_info;
    delete    pflex_unused;
    delete [] pnew_file;
} // free_memory

// if file won't fit return 0 otherwise return 1
Byte NafsDirectoryContainer::add_to_link_table(
    SWord dir_index,
    off_t size,
    Byte random,
    t_st *pbegin,
    t_st *pend)
{
    Word i, free, begin, records;
    struct s_link_table *plink;
    struct s_sys_info_sector *psis;

    psis = pflex_sys_info;
    free = (psis->free[0] << 8) + psis->free[1];

    if (size > (off_t)(free * 252L))
    {
        return 0;
    }

    records = (size + 251L) / 252;
    pbegin->st.sec = psis->fc_start_sec;
    pbegin->st.trk = psis->fc_start_trk;
    begin = (pbegin->st.trk * MAX_SECTOR) + pbegin->st.sec - 1;

    for (i = 1; i <= records; i++)
    {
        plink = pflex_links + i + begin - 1;

        if (i == records)
        {
            plink->next.sec_trk = 0;
        }

        if (random)
        {
            plink->record_nr[0] = i > 2 ? (i - 2) >> 8 : 0;
            plink->record_nr[1] = i > 2 ? (i - 2) & 0xff : 0;
        }
        else
        {
            plink->record_nr[0] = i >> 8;
            plink->record_nr[1] = i & 0xff;
        }

        plink->f_record = i - 1;
        plink->file_id = dir_index;
    } // for

    pend->st.sec = ((i + begin - 2) % MAX_SECTOR) + 1;
    pend->st.trk = (i + begin - 2) / MAX_SECTOR;
    // update sys info sector
    psis->fc_start_sec = ((i + begin - 1) % MAX_SECTOR) + 1;
    psis->fc_start_trk = (i + begin - 1) / MAX_SECTOR;
    psis->free[0] = (free - records) >> 8;
    psis->free[1] = (free - records) & 0xff;
    return 1;
} // add_to_link_table


void NafsDirectoryContainer::add_to_directory(
    char *name,
    char *ext,
    SWord dir_index,
    Byte random,
    struct stat *pstat,
    t_st *pbegin,
    t_st *pend,
    Byte wp)
{
    s_dir_entry *pd;
    struct tm *lt;
    SWord records;

    lt = localtime(&(pstat->st_mtime));
    records = (pstat->st_size + 251L) / 252;
    pd = &((pflex_directory + (dir_index / 10))->dir_entry[dir_index % 10]);
    memset(pd->filename, 0, FLEX_BASEFILENAME_LENGTH);
    strncpy(pd->filename, name, FLEX_BASEFILENAME_LENGTH);
    memset(pd->file_ext, 0, FLEX_FILEEXT_LENGTH);
    strncpy(pd->file_ext, ext, FLEX_FILEEXT_LENGTH);
    pd->file_attr   = wp ? 0xc0 : 0x00;
    pd->start_trk   = pbegin->st.trk;
    pd->start_sec   = pbegin->st.sec;
    pd->end_trk     = pend->st.trk;
    pd->end_sec     = pend->st.sec;
    pd->records[0]  = records >> 8;
    pd->records[1]  = records & 0xff;
    pd->sector_map  = random ? 0x02 : 0x00;
    pd->month       = lt->tm_mon + 1;
    pd->day         = lt->tm_mday;
    pd->year        = lt->tm_year;
} // add_to_directory

bool NafsDirectoryContainer::is_in_file_random(const char *ppath,
        const char *pfilename)
{
    char str[PATH_MAX + 1];

    strcpy(str, ppath);
    strcat(str, PATHSEPARATORSTRING RANDOM_FILE_LIST);

    BFilePtr fp(str, "r");

    if (fp != NULL)
    {
        while (!feof((FILE *)fp))
        {
            fgets(str, PATH_MAX, fp);

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

void NafsDirectoryContainer::modify_random_file(char *path, struct stat *pstat,
        t_st *pbegin)
{
    Byte file_sector_map[252 * 2];
    SDWord data_size;
    Word i, n, index;

    data_size = pstat->st_size - (252L * 2);

    if (data_size >= 252L * 2)
    {
        index = (pbegin->st.trk * MAX_SECTOR + pbegin->st.sec) - 1 + 2;

        for (i = 0; i < 252 * 2; i++)
        {
            file_sector_map[i] = 0;
        }

        n = 0;

        for (n = 0; n < (data_size / (252L * 255)) ; n++)
        {
            file_sector_map[3 * n]   = index / MAX_SECTOR;
            file_sector_map[3 * n + 1] = (index % MAX_SECTOR) + 1;
            file_sector_map[3 * n + 2] = 255;
            index += 255;
        } // for

        i = (Word)(data_size % (252L * 255));

        if (i != 0)
        {
            file_sector_map[3 * n]   = index / MAX_SECTOR;
            file_sector_map[3 * n + 1] = (index % MAX_SECTOR) + 1;
            file_sector_map[3 * n + 2] = (i + 251) / 252;
        } // if

        BFilePtr fp(path, "rb+");

        if (fp != NULL)
        {
            fwrite(&file_sector_map, 252, 2, fp);
        }
    } // if
} // modify_random_file

// dwp = write protect for drive

void NafsDirectoryContainer::fill_flex_directory(Byte dwp)
{
#ifdef WIN32
    HANDLE hdl;
    WIN32_FIND_DATA pentry;
#endif
#ifdef UNIX
    DIR *pd;
    struct dirent *pentry;
#endif
    char name[9], ext[4], path[PATH_MAX + 1], *pfilename;
    SWord dir_index = 0;
    Word random;
    t_st begin, end;
    struct stat sbuf;

    initialize_flex_directory();
    initialize_flex_link_table();
#ifdef WIN32
    strcpy(path, *dir);
    strcat(path, PATHSEPARATORSTRING "*.*");

    if ((hdl = FindFirstFile(path, &pentry)) != INVALID_HANDLE_VALUE)
    {
        do
        {
#endif
#ifdef UNIX

            if ((pd = opendir(dir->c_str())) != NULL)
            {
                while ((pentry = readdir(pd)) != NULL)
                {
#endif
#ifdef WIN32
                    pfilename = pentry.cFileName;
                    strlower(pfilename);
#endif
#ifdef UNIX
                    pfilename = (char *)&pentry->d_name;
#endif
                    random = 0;
                    strcpy((char *)&path, dir->c_str());
                    strcat((char *)&path, PATHSEPARATORSTRING);
                    strcat((char *)&path, pfilename);

                    if (IsFlexFilename(pfilename, (char *)&name,
                                       (char *)&ext) &&
                        !stat(path, &sbuf) && (S_ISREG(sbuf.st_mode)) &&
                        // exclude file "random:"
                        strcmp(pfilename, RANDOM_FILE_LIST) &&
                        sbuf.st_size > 0 &&
                        (dir_index = next_free_dir_entry()) >= 0)
                    {
#ifdef WIN32
                        random = (pentry.dwFileAttributes &
                                  FILE_ATTRIBUTE_HIDDEN) ? 1 : 0;
#endif
#ifdef UNIX
                        random = sbuf.st_mode & S_IXUSR;
#endif

                        // CDFS-Support: look for file name in file 'random'
                        if (dwp)
                        {
                            random = is_in_file_random(dir->c_str(), pfilename);
                        }

                        if (add_to_link_table(dir_index, sbuf.st_size,
                                              (Byte)random, &begin, &end))
                        {
                            add_to_directory(name, ext, dir_index,
                                             (Byte)random, &sbuf, &begin, &end,
                                             dwp || access(path, W_OK));

                            // Unfinished: don't write sector map if write
                            // protected
                            if (random && !dwp)
                                modify_random_file(path,
                                                   &sbuf, &begin);
                        }
                    }

#ifdef WIN32
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
        struct s_sys_info_sector *psis;

        if (!stat(dir->c_str(), &sbuf))
        {
            psis = pflex_sys_info;
            lt = localtime(&(sbuf.st_mtime));
            name[0] = '\0';
            ext[0]  = '\0';
            pName = dir->c_str() + dir->length() - 1;

            if (*pName == PATHSEPARATOR)
            {
                pName--;
            }

            while (pName != dir->c_str() && *pName != PATHSEPARATOR)
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

            strncpy((char *) & (psis->disk_name), name, 8);
            strncpy((char *) & (psis->disk_ext), ext, 3);
            psis->disk_number[0] = number >> 8;
            psis->disk_number[1] = number & 0xff;
            psis->fc_start_trk = 0;
            psis->fc_start_sec = 0;
            psis->fc_end_trk = 0;
            psis->fc_end_sec = 0;
            psis->free[0] = 0;
            psis->free[1] = 0;
            psis->month = lt->tm_mon + 1;
            psis->day = lt->tm_mday;
            psis->year = lt->tm_year;
            psis->last_trk = MAX_TRACK;
            psis->last_sec = MAX_SECTOR;

            for (i = 0; i < 216; i++)
            {
                psis->unused2[i] = 0;
            }

            memcpy(pflex_sys_info + 1, pflex_sys_info,
                   sizeof(struct s_sys_info_sector));
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

        while (i >= 0 && (pflex_links + i)->file_id == old_id)
        {
            (pflex_links + i)->file_id = new_id;
            i = (pflex_links + i)->next.st.trk * MAX_SECTOR +
                (pflex_links + i)->next.st.sec - 1;
        } // while
    } // change_file_id


    void NafsDirectoryContainer::check_for_delete(SWord dir_index,
            s_dir_sector * pdb) const
    {
        Word i;
        SWord index;
        char path[PATH_MAX + 1];
        const char *pfilename;
        s_dir_sector *pd;

        pd = pflex_directory + dir_index;

        for (i = 0; i < 10; i++)
        {
            if (pdb->dir_entry[i].filename[0] == -1 &&
                pd->dir_entry[i].filename[0] != -1)
            {
                pfilename = unix_filename(dir_index * 10 + i).c_str();
                index = pd->dir_entry[i].start_trk * MAX_SECTOR +
                        pd->dir_entry[i].start_sec - 1;
                strcpy((char *)&path, dir->c_str());
                strcat((char *)&path, PATHSEPARATORSTRING);
                strcat((char *)&path, pfilename);
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
            s_dir_sector * pdb) const
    {
        Word    i;
        char    old_path[PATH_MAX + 1], new_path[PATH_MAX + 1];
        const char    *pfilename1, *pfilename2;

        for (i = 0; i < 10; i++)
        {
            pfilename1 = unix_filename(dir_index * 10 + i).c_str();
            pfilename2 = get_unix_filename(pdb->dir_entry[i].filename).c_str();

            if (*pfilename1 && *pfilename2 &&
                strcmp(pfilename1, pfilename2) != 0)
            {
                strcpy((char *)&old_path, dir->c_str());
                strcat((char *)&old_path, PATHSEPARATORSTRING);
                strcat((char *)&old_path, pfilename1);
                strcpy((char *)&new_path, dir->c_str());
                strcat((char *)&new_path, PATHSEPARATORSTRING);
                strcat((char *)&new_path, pfilename2);
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
            s_dir_sector * pdb)
    {
        s_dir_sector *pd;

        pd = pflex_directory + dir_index;

        if (!pd->next_sec && !pd->next_trk && (pdb->next_trk || pdb->next_sec))
        {
            dir_extend.st.trk = pdb->next_trk;
            dir_extend.st.sec = pdb->next_sec;
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


    Byte NafsDirectoryContainer::check_for_new_file(SWord dir_index,
            s_dir_sector * pd) const
    {
        Word i, j;
        SWord index;
        char old_path[PATH_MAX + 1], new_path[PATH_MAX + 1];
#ifdef UNIX
        struct stat sbuf;
#endif

        j = 0;

        while ((pnew_file + j)->first.sec_trk && j < new_files)
        {
            for (i = 0; i < 10; i++)
            {
                if ((pnew_file + j)->first.st.sec ==
                    pd->dir_entry[i].start_sec &&
                    (pnew_file + j)->first.st.trk == pd->dir_entry[i].start_trk)
                {
                    index = (pnew_file + j)->first.st.trk * MAX_SECTOR +
                            (pnew_file + j)->first.st.sec - 1;
                    change_file_id(index, NEW_FILE1 - j,
                                   10 * dir_index + i);
                    fclose((pnew_file + j)->fp);
                    strcpy((char *)&old_path, dir->c_str());
                    strcat((char *)&old_path, PATHSEPARATORSTRING);
                    strcat((char *)&old_path, (pnew_file + j)->filename);

                    // check for random file, if true set user execute bit
                    if (pd->dir_entry[i].sector_map & 0x02)
#ifdef WIN32
                        SetFileAttributes(old_path, FILE_ATTRIBUTE_HIDDEN);

#endif
#ifdef UNIX

                    if (!stat(old_path, &sbuf))
                    {
                        chmod(old_path, sbuf.st_mode | S_IXUSR);
                    }

#endif
                    strcpy((char *)&new_path, dir->c_str());
                    strcat((char *)&new_path, PATHSEPARATORSTRING);
                    strcat((char *)&new_path, unix_filename(
                               (pflex_links + index)->file_id).c_str());
                    rename(old_path, new_path);
#ifdef DEBUG_FILE
                    LOG_XX("     new file %s, was %s\n",
                           unix_filename((pflex_links + index)->file_id),
                           (pnew_file + j)->filename);
#endif
                    set_file_time(new_path, pd->dir_entry[i].month,
                                  pd->dir_entry[i].day, pd->dir_entry[i].year);

                    // remove entry in new file table and
                    // move following entries
                    while ((pnew_file + j + 1)->first.sec_trk &&
                           j < new_files - 1)
                    {
                        strcpy((char *) & (pnew_file + j)->filename,
                               (pnew_file + j + 1)->filename);
                        (pnew_file + j)->first.sec_trk =
                            (pnew_file + j + 1)->first.sec_trk;
                        (pnew_file + j)->next.sec_trk  =
                            (pnew_file + j + 1)->next.sec_trk;
                        (pnew_file + j)->f_record =
                            (pnew_file + j + 1)->f_record;
                        (pnew_file + j)->fp = (pnew_file + j + 1)->fp;
                        index = (pnew_file + j)->first.st.trk * MAX_SECTOR +
                                (pnew_file + j)->first.st.sec - 1;
                        change_file_id(index, NEW_FILE1 - j - 1, NEW_FILE1 - j);
                        j++;
                    } // while

                    (pnew_file + j)->first.sec_trk = 0;
                    return 0;
                } // if
            } // for

            j++;
        } // while

        return 0;
    } // check_for_new_file


    Byte NafsDirectoryContainer::last_of_free_chain(Byte trk, Byte sec) const
    {
        return (pflex_sys_info->fc_end_trk == trk &&
                pflex_sys_info->fc_end_sec == sec);
    } // last_of_free_chain


    // return true on success
    bool NafsDirectoryContainer::ReadSector(Byte * buffer, int trk,
                                            int sec) const
    {
        char *p, path[PATH_MAX + 1];
        const char *mode;
        Word i, di;
        size_t bytes;
        FILE *fp;
        t_st *link;
        struct s_link_table *pfl;
        bool result;

#ifdef DEBUG_FILE
        LOG_XX("read: %02X/%02X ", trk, sec);
#endif
        mode = "rb";
        result = true;
        i = trk * MAX_SECTOR + (sec - 1);
        pfl = pflex_links + i;

        switch (pfl->file_id)
        {
            case SYSTEM :
#ifdef DEBUG_FILE
                LOG("systemsector\n");
#endif
                p = NULL; // satisfy compiler

                if (sec >= 3)
                {
                    p = (char *)(pflex_sys_info + sec - 3);
                }
                else if (sec == 2)
                {
                    p = (char *)pflex_unused;
                }
                else if (sec == 1)
                {
                    bytes = SECTOR_SIZE;

                    for (i = 0; i < SECTOR_SIZE; i++)
                    {
                        buffer[i] = 0;
                    }

                    buffer[0] = 0x39; // means RTS
                    link = link_address();
                    strcpy((char *)path, dir->c_str());
                    strcat((char *)path, PATHSEPARATORSTRING "boot");

                    if ((fp = fopen(path, mode)) != NULL)
                    {
                        bytes = fread(buffer, 1, SECTOR_SIZE, fp);
                        fclose(fp);
                    }

                    if (bytes != SECTOR_SIZE)
                    {
                        result = false;
                    }

                    buffer[3] = link->st.trk;
                    buffer[4] = link->st.sec;
                    break;
                }
                else
                {
                    break;
                }

                memcpy(buffer, p, param.byte_p_sector);
                break;

            case DIRECTORY:
#ifdef DEBUG_FILE
                LOG("directorysector\n");
#endif
                di = pfl->f_record;
                p = (char *)(pflex_directory + di);
                memcpy(buffer, p, param.byte_p_sector);
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
                for (i = 4; i < SECTOR_SIZE; i++)
                {
                    buffer[i] = 0;
                }

                break;

            default :
                if (pfl->file_id >= 0)
                {
#ifdef DEBUG_FILE
                    LOG_X("sector of file %s\n", unix_filename(pfl->file_id));
#endif
                    strcpy((char *)&path, dir->c_str());
                    strcat((char *)&path, PATHSEPARATORSTRING);
                    strcat((char *)&path, unix_filename(pfl->file_id).c_str());

                    if ((fp = fopen(path, mode)) != NULL &&
                        !fseek(fp, (long)(pfl->f_record * 252L),
                               SEEK_SET))
                    {
                        bytes = fread(&buffer[4], 1, 252, fp);
                        fclose(fp);

                        // stuff last sector of file with 0
                        for (i = 4 + bytes; i < SECTOR_SIZE; i++)
                        {
                            buffer[i] = 0;
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
                    fp = (pnew_file + (NEW_FILE1 - pfl->file_id))->fp;

                    if (!fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET))
                    {
                        bytes = fread(&buffer[4], 1, 252, fp);

                        // stuff last sector of file with 0
                        for (i = 4 + bytes; i < SECTOR_SIZE; i++)
                        {
                            buffer[i] = 0;
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
        char *p, path[PATH_MAX + 1];
        const char *mode;
        Word i, di;
        size_t bytes;
        SWord new_file_index;
        FILE *fp;
        struct s_link_table *pfl;
        bool result;

#ifdef DEBUG_FILE
        LOG_XX("write: %02X/%02X ", trk, sec);
#endif
        fp = NULL;
        mode = "rb+";
        result = 1;
        bytes = 0;      // satisfy compiler
        i = trk * MAX_SECTOR + (sec - 1);
        pfl = pflex_links + i;

        switch (pfl->file_id)
        {
            case SYSTEM :
#ifdef DEBUG_FILE
                LOG("systemsector\n");
#endif
                p = NULL; // satisfy compiler

                if (sec >= 3)
                {
                    p = (char *)(pflex_sys_info + sec - 3);
                }
                else if (sec == 2)
                {
                    p = (char *)pflex_unused;
                }
                else if (sec == 1)
                {
                    mode = "wb";
                    strcpy((char *)&path, dir->c_str());
                    strcat((char *)&path, PATHSEPARATORSTRING "boot");

                    if ((fp = fopen(path, mode)) != NULL)
                    {
                        bytes = fwrite(&buffer, 1, SECTOR_SIZE, fp);
                        fclose(fp);
                    }

                    if (bytes != SECTOR_SIZE)
                    {
                        result = false;
                    }

                    break;
                }
                else
                {
                    break;
                }

                memcpy(p, buffer, param.byte_p_sector);
                break;

            case DIRECTORY:
#ifdef DEBUG_FILE
                LOG("directorysector\n");
#endif
                di = pfl->f_record;
                p = (char *)(pflex_directory + di);
                check_for_delete(di, (s_dir_sector *)buffer);
                check_for_new_file(di, (s_dir_sector *)buffer);
                check_for_rename(di, (s_dir_sector *)buffer);
                check_for_extend(di, (s_dir_sector *)buffer);
                memcpy(p, buffer, param.byte_p_sector);
                break;

            case FREE_CHAIN :
#ifdef DEBUG_FILE
                LOG("free chain\n");
#endif

                if (dir_extend.st.trk == trk && dir_extend.st.sec == sec)
                {
                    extend_directory(i, (s_dir_sector *)buffer);
#ifdef DEBUG_FILE
                    LOG("      extend directory\n");
#endif
                    break;
                } // if

                if (last_of_free_chain(trk, sec) && (buffer[1] || buffer[0]))
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

                if ((new_file_index = index_of_new_file(trk, sec)) < 0)
                {
#ifdef DEBUG_FILE
                    LOG("   ** error: unable to create new file\n");
#endif
                    result = false; // no more new files can be created
                    break;
                }

#ifdef DEBUG_FILE
                LOG_X("      file %s\n",
                      (pnew_file + new_file_index)->filename);
#endif
                fp = (pnew_file + new_file_index)->fp;
                (pnew_file + new_file_index)->next.st.trk = buffer[0];
                (pnew_file + new_file_index)->next.st.sec = buffer[1];
                pfl->file_id  = NEW_FILE1 - new_file_index;
                //pfl->f_record = (pfs->pnew_file+new_file_index)->f_record++;
                //pfl->f_record = ((sector_buffer[2] << 8) |
                // sector_buffer[3]) - 1;
                pfl->next.st.trk = buffer[0];
                pfl->next.st.sec = buffer[1];
                pfl->record_nr[0] = buffer[2];
                pfl->record_nr[1] = buffer[3];
                pfl->f_record = record_nr_of_new_file(new_file_index, i);

                if (ftell(fp) != (pfl->f_record * 252L) &&
                    fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET) != 0)
                {
                    result = false;
                }
                else if ((bytes = fwrite(&buffer[4], 1, 252, fp)) != 252)
                {
                    result = false;
                }

                // (pfs->pnew_file+new_file_index)->f_record++;
                break;

            default :
                if (pfl->file_id >=  0)
                {
#ifdef DEBUG_FILE
                    LOG_X("sector of file %s\n", unix_filename(pfl->file_id));
#endif
                    strcpy((char *)&path, dir->c_str());
                    strcat((char *)&path, PATHSEPARATORSTRING);
                    strcat((char *)&path, unix_filename(pfl->file_id).c_str());
                    pfl->next.st.trk = buffer[0];
                    pfl->next.st.sec = buffer[1];
                    pfl->record_nr[0] = buffer[2];
                    pfl->record_nr[1] = buffer[3];

                    if ((fp = fopen(path, mode)) == NULL)
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
                        else if ((bytes = fwrite(&buffer[4], 1, 252,
                                                 fp)) != 252)
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
                    fp = (pnew_file + (NEW_FILE1 - pfl->file_id))->fp;

                    if (ftell(fp) != pfl->f_record &&
                        fseek(fp, (long)(pfl->f_record * 252L), SEEK_SET) != 0)
                    {
                        result = false;
                    }
                    else if ((bytes = fwrite(&buffer[4], 1, 252, fp)) != 252)
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
        int wp; // should be int because of call 'access'

        wp = access(dir->c_str(), W_OK);
        initialize_header(wp);
        initialize_new_file_table();
        initialize_flex_sys_info_sectors(number);
        initialize_flex_unused_sector();
        fill_flex_directory(wp);
    } // mount

#endif // #ifdef NAFS
