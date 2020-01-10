/*
    ffilecnt.cpp


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
#include <sys/stat.h>
#include <string>

#include "fcinfo.h"
#include "flexerr.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "bdate.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "ifilecnt.h"
#include "ifilcnti.h"
#include "iffilcnt.h"

#ifdef UNIX
    std::string FlexFileContainer::bootSectorFile =
        F_DATADIR PATHSEPARATORSTRING BOOT_FILE;
#endif
#ifdef _WIN32
    std::string FlexFileContainer::bootSectorFile = BOOT_FILE;
#endif

/***********************************************/
/* Initialization of a s_flex_header structure */
/***********************************************/

void s_flex_header::initialize(int secsize, int trk, int sec0, int sec,
                               int aSides)
{
    Byte i, size, noSides;

    noSides = static_cast<Byte>(aSides);

    if (aSides < 1)
    {
        noSides = 1;
    }

    if (aSides > 2)
    {
        noSides = 2;
    }

    if (trk > 255)
    {
        trk = 255;
    }

    if (sec0 > 255)
    {
        sec0 = 255;
    }

    if (sec > 255)
    {
        sec = 255;
    }

    size = 1; /* default */

    for (i = 15; i >= 7; i--)
    {
        if (secsize & (1 << i))
        {
            size = i - 7;
            break;
        }
    }

    magic_number    = MAGIC_NUMBER;
    write_protect   = 0;
    sizecode        = size;
    sides0          = noSides;
    sectors0        = static_cast<Byte>(sec0);
    sides           = noSides;
    sectors         = static_cast<Byte>(sec);
    tracks          = static_cast<Byte>(trk);
    dummy1          = 0;
    dummy2          = 0;
    dummy3          = 0;
    dummy4          = 0;
    dummy5          = 0;
}


/****************************************/
/* Constructor                          */
/****************************************/

FlexFileContainer::FlexFileContainer(const char *path, const char *mode) :
    fp(path, mode), attributes(0)
{
    struct  stat sbuf;
    struct  s_flex_header header;

    if (fp == nullptr)
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    if (fseek(fp, 0, SEEK_SET))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    if (strchr(fp.GetMode(), '+') == nullptr)
    {
        attributes |= FLX_READONLY;
    }

    // try to read the FLX header
    // to check if it is a FLX formated disk
    if (fread(&header, sizeof(header), 1, fp) == 1 &&
        header.magic_number == MAGIC_NUMBER)
    {
        // ok it's a FLX format
        Initialize_for_flx_format(&param, &header,
                                  (attributes & FLX_READONLY) ? true : false);
        return;
    }
    else
    {
        s_formats   format;
        s_sys_info_sector sis;

        // check if it is a DSK formated disk
        // read system info sector
        if (fseek(fp, 2 * SECTOR_SIZE, SEEK_SET))
        {
            throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
        }

        if (fread(&sis, sizeof(sis), 1, fp) == 1)
        {
            format.tracks = sis.sir.last.trk + 1;
            format.sectors = sis.sir.last.sec;
            format.size     = format.tracks * format.sectors * SECTOR_SIZE;

            // do a plausibility check with the size of the DSK file
            if (!stat(fp.GetPath(), &sbuf) &&
                format.size == sbuf.st_size)
            {
                // ok it's a DSK format
                Initialize_for_dsk_format(&param, &format,
                                          (attributes & FLX_READONLY) ?
                                          true : false);
                return;
            }
        }
    }

    throw FlexException(FERR_IS_NO_FILECONTAINER, fp.GetPath());
}

/****************************************/
/* Destructor                           */
/****************************************/

FlexFileContainer::~FlexFileContainer()
{
}

FlexFileContainer::FlexFileContainer(FlexFileContainer &&src) :
    fp(std::move(src.fp)), param(src.param), attributes(src.attributes)
{
}

FlexFileContainer &FlexFileContainer::operator= (FlexFileContainer &&src)
{
    fp = std::move(src.fp);
    param = src.param;
    attributes = src.attributes;

    return *this;
}

/****************************************/
/* Public interface                     */
/****************************************/

std::string FlexFileContainer::GetPath() const
{
    return fp.GetPath();
}

int FlexFileContainer::GetBytesPerSector() const
{
    return param.byte_p_sector;
}

bool FlexFileContainer::IsWriteProtected() const
{
    return (param.write_protect ? true : false);
}

bool FlexFileContainer::IsTrackValid(int track) const
{
    return (track >= 0 && track <= param.max_track);
}

bool FlexFileContainer::IsSectorValid(int track, int sector) const
{
    if (track)
    {
        return (sector > 0 && sector <= param.max_sector);
    }
    else
    {
        return (sector > 0 && sector <= param.max_sector0);
    }
}

FlexFileContainer *FlexFileContainer::Create(const char *dir, const char *name,
        int t, int s, int fmt)
{
    std::string path;

    if (fmt != TYPE_DSK_CONTAINER && fmt != TYPE_FLX_CONTAINER)
    {
        throw FlexException(FERR_INVALID_FORMAT, fmt);
    }

    Format_disk(t, s, dir, name, fmt);

    path = dir;

    if (!path.empty() && path[path.length()-1] != PATHSEPARATOR)
    {
        path += PATHSEPARATORSTRING;
    }

    path += name;
    return new FlexFileContainer(path.c_str(), "rb+");
}

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool FlexFileContainer::FindFile(const char *fileName, FlexDirEntry &entry)
{
    FileContainerIterator it(fileName);

    it = this->begin();

    if (it == this->end())
    {
        entry.SetEmpty();
        return false;
    }

    entry = *it;
    return true;
}

bool    FlexFileContainer::DeleteFile(const char *filePattern)
{
    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(filePattern);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool    FlexFileContainer::RenameFile(const char *oldName, const char *newName)
{
    CHECK_CONTAINER_WRITEPROTECTED;

    FlexDirEntry de;

    // prevent overwriting of an existing file
    if (FindFile(newName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    FileContainerIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, fp.GetPath());
    }
    else
    {
        it.RenameCurrent(newName);
    }

    return true;
}

bool FlexFileContainer::FileCopy(const char *sourceName, const char *destName,
                                 FileContainerIf &destination)
{
    FlexCopyManager copyMan;

    return copyMan.FileCopy(sourceName, destName, *this, destination);
}

bool    FlexFileContainer::GetInfo(FlexContainerInfo &info) const
{
    s_sys_info_sector sis;
    char disk_name[13];
    int year;

    if (!ReadSector(reinterpret_cast<Byte *>(&sis), 0, 3))
    {
        throw FlexException(FERR_READING_TRKSEC, 0, 3, fp.GetPath());
    }

    if (sis.sir.year < 75)
    {
        year = sis.sir.year + 2000;
    }
    else
    {
        year = sis.sir.year + 1900;
    }

    strncpy(disk_name, sis.sir.disk_name, sizeof(sis.sir.disk_name));
    disk_name[sizeof(sis.sir.disk_name)] = '\0';
    if (sis.sir.disk_ext[0] != '\0')
    {
        strcat(disk_name, ".");
        size_t index = strlen(disk_name);
        for (size_t i = 0; i < sizeof(sis.sir.disk_ext); ++i)
        {
            char ch = sis.sir.disk_ext[i];
            if (ch >= ' ' && ch <= '~')
            {
                disk_name[index++] = ch;
            }
            else
            {
                break;
            }
        }
        disk_name[index++] = '\0';
    }
    info.SetDate(sis.sir.day, sis.sir.month, year);
    info.SetTrackSector(sis.sir.last.trk + 1, sis.sir.last.sec);
    info.SetFree((getValueBigEndian<Word>(&sis.sir.free[0]) * param.byte_p_sector) >> 10);
    info.SetTotalSize(((sis.sir.last.sec * (sis.sir.last.trk + 1)) *
                       param.byte_p_sector) >> 10);
    info.SetName(disk_name);
    info.SetNumber(getValueBigEndian<Word>(&sis.sir.disk_number[0]));
    info.SetPath(fp.GetPath());
    info.SetType(param.type);
    info.SetAttributes(attributes);
    return true;
}

int FlexFileContainer::GetContainerType() const
{
    return param.type;
}

bool FlexFileContainer::CheckFilename(const char *fileName) const
{
    int     result; // result from sscanf should be int
    char    dot;
    char    name[9];
    char    ext[4];
    const char    *format;

    dot    = '\0';
    format = "%1[A-Za-z]%7[A-Za-z0-9_-]";
    result = sscanf(fileName, format, name, &name[1]);

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

    result = sscanf(fileName, format, &dot, ext, &ext[1]);

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != strlen(fileName))
    {
        return false;
    }

    return true;
}

/******************************/
/* Nonpublic interface        */
/******************************/

FileContainerIteratorImpPtr FlexFileContainer::IteratorFactory()
{
    return FileContainerIteratorImpPtr(new FlexFileContainerIteratorImp(this));
}

// if successfull return true. If error return false
bool FlexFileContainer::WriteFromBuffer(const FlexFileBuffer &buffer,
                                        const char *fileName /* = nullptr */)
{
    Byte trk = 0, sec = 0;
    st_t start;
    st_t next;
    Word recordNr;
    int count;
    FlexDirEntry    de;
    s_sys_info_sector sis;
    const char      *pFileName = fileName;
    // sectorBuffer[2] and [1] are used for the Sector Map
    Byte sectorBuffer[3][SECTOR_SIZE];

    if (fileName == nullptr)
    {
        pFileName = buffer.GetFilename();
    }

    if (FindFile(pFileName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, pFileName);
    }

    // read sys info sector
    if (!ReadSector(reinterpret_cast<Byte *>(&sis), 0, 3))
    {
        throw FlexException(FERR_READING_TRKSEC, 0, 3, fp.GetPath());
    } // get start trk/sec of free chain

    next = start = sis.sir.fc_start;
    {
        // write each sector to buffer
        Byte repeat = 0;
        unsigned int smIndex,  smSector;
        Word nextPTrk, nextPSec; // contains next physical trk/sec
        smIndex = 1;
        smSector = 2;
        recordNr = nextPTrk = nextPSec = 0;

        // at the begin of a random file reserve two sectors for the sector map
        if (recordNr == 0 && buffer.IsRandom())
        {
            repeat = 2;
        }

        do
        {
            for (count = repeat; count >= 0; count--)
            {
                trk = next.trk;
                sec = next.sec;

                if (trk == 0 && sec == 0)
                {
                    // disk full
                    throw FlexException(FERR_DISK_FULL_WRITING,
                                        fp.GetPath(), pFileName);
                }

                if (!ReadSector(&sectorBuffer[count][0], trk, sec))
                {
                    throw FlexException(FERR_READING_TRKSEC,
                                        trk, sec, fp.GetPath());
                }
                else if (count)
                {
                    memset(&sectorBuffer[count][2], 0, SECTOR_SIZE - 2);
                }

                next.trk = sectorBuffer[count][0];
                next.sec = sectorBuffer[count][1];
            }

            if (!buffer.CopyTo(&sectorBuffer[0][4], SECTOR_SIZE - 4,
                               recordNr * (SECTOR_SIZE - 4), 0x00))
            {
                throw FlexException(FERR_WRITING_TRKSEC,
                                    trk, sec, fp.GetPath());
            }

            recordNr++;

            // if random file update sector map
            if (buffer.IsRandom())
            {
                if (trk != nextPTrk || sec != nextPSec ||
                    sectorBuffer[smSector][smIndex + 2] == 255)
                {
                    smIndex += 3;

                    if (smIndex >= SECTOR_SIZE)
                    {
                        smSector--;

                        if (smSector == 0)
                        {
                            throw FlexException(FERR_RECORDMAP_FULL,
                                                pFileName, fp.GetPath());
                        }

                        smIndex = 4;
                    }

                    sectorBuffer[smSector][smIndex] = trk;
                    sectorBuffer[smSector][smIndex + 1] = sec;
                }

                sectorBuffer[smSector][smIndex + 2]++;
                nextPTrk = trk;

                if ((nextPSec = sec + 1) > (param.byte_p_track /
                                            param.byte_p_sector))
                {
                    nextPTrk++;
                    nextPSec = 1;
                }
            }

            // set record nr and if last sector set link to 0. Write sector
            setValueBigEndian<Word>(&sectorBuffer[0][2], recordNr);

            if (recordNr * (SECTOR_SIZE - 4) >= buffer.GetFileSize())
            {
                sectorBuffer[0][0] = sectorBuffer[0][1] = 0;
            }

            if (!WriteSector(&sectorBuffer[0][0], trk, sec))
            {
                throw FlexException(FERR_WRITING_TRKSEC,
                                    trk, sec, fp.GetPath());
            }

            repeat = 0;
        }
        while (recordNr * (SECTOR_SIZE - 4) < buffer.GetFileSize());
    }
    sis.sir.fc_start = next;

    // if free chain full, set end trk/sec of free chain also to 0
    if (!next.sec && !next.trk)
    {
        sis.sir.fc_end = next;
    }

    // if random file, write the sector map buffers back
    next = start;

    if (buffer.IsRandom())
    {
        for (count = 2; count >= 1; count--)
        {
            if (!WriteSector(&sectorBuffer[count][0], next.trk, next.sec))
            {
                throw FlexException(FERR_WRITING_TRKSEC,
                                    next.trk, next.sec, fp.GetPath());
            }

            next.trk = sectorBuffer[count][0];
            next.sec = sectorBuffer[count][1];
        }
    }

    // update sys info sector
    auto free = getValueBigEndian<Word>(&sis.sir.free[0]);
    free -= recordNr;
    setValueBigEndian<Word>(&sis.sir.free[0], free);

    if (!WriteSector(reinterpret_cast<Byte *>(&sis), 0, 3))
    {
        throw FlexException(FERR_WRITING_TRKSEC, 0, 3, fp.GetPath());
    }

    // make new directory entry
    de.SetDate(buffer.GetDate());
    de.SetStartTrkSec(start.trk, start.sec);
    de.SetEndTrkSec(trk, sec);
    de.SetTotalFileName(pFileName);
    de.SetSize(recordNr * SECTOR_SIZE);
    de.SetAttributes(buffer.GetAttributes());
    de.SetSectorMap(buffer.GetSectorMap());
    CreateDirEntry(de);
    return true;
}

FlexFileBuffer FlexFileContainer::ReadToBuffer(const char *fileName)
{
    FlexFileBuffer  buffer;
    FlexDirEntry    de;
    int             trk, sec;
    int             recordNr, repeat;
    Byte            sectorBuffer[SECTOR_SIZE];
    int             size;

    if (!FindFile(fileName, de))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fileName);
    }

    buffer.SetAttributes(de.GetAttributes());
    buffer.SetSectorMap(de.GetSectorMap());
    buffer.SetFilename(fileName);
    buffer.SetDate(de.GetDate());
    size = de.GetSize();

    if (de.IsRandom())
    {
        size -= 2 * SECTOR_SIZE;
    }

    if (size < 0)
    {
        throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                            std::to_string(size / SECTOR_SIZE));
    }

    size = size * DBPS / SECTOR_SIZE;
    buffer.Realloc(size);
    recordNr = 0;
    repeat = 1;

    if (size > 0)
    {
        de.GetStartTrkSec(trk, sec);

        while (true)
        {
            // if random file skip the two sector map sectors
            if (recordNr == 0 && de.IsRandom())
            {
                repeat = 3;
            }

            for (int i = 0; i < repeat; i++)
            {
                if (trk == 0 && sec == 0)
                {
                    return buffer;
                }
                if (!ReadSector(&sectorBuffer[0], trk, sec))
                {
                    throw FlexException(FERR_READING_TRKSEC,
                                        trk, sec, fileName);
                }

                trk = sectorBuffer[0];
                sec = sectorBuffer[1];
            } // for

            if (!buffer.CopyFrom(&sectorBuffer[4], SECTOR_SIZE - 4,
                                 recordNr * (SECTOR_SIZE - 4)))
            {
                size = recordNr + 1;
                throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                                    std::to_string(size));
            }

            recordNr++;
            repeat = 1;
        }
    }

    return buffer;
}


// set the file attributes of one or multiple files
bool    FlexFileContainer::SetAttributes(const char *filePattern,
        Byte setMask, Byte clearMask)
{
    FlexDirEntry de;

    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(filePattern);

    for (it = this->begin(); it != this->end(); ++it)
    {
        Byte x_attributes = (it->GetAttributes() & ~clearMask) | setMask;
        it.SetAttributesCurrent(x_attributes);
    }

    return true;
}

bool FlexFileContainer::CreateDirEntry(FlexDirEntry &entry)
{

    int     i;
    s_dir_sector    ds;
    s_dir_entry *pde;
    st_t next{0, 5}; // first directory sector is trk/sec 0/5
    int     tmp1, tmp2;
    BDate       date;

    // loop until all directory sectors read
    while (next.sec != 0 || next.trk != 0)
    {
        // read next directory sector
        if (!ReadSector((Byte *)&ds, next.trk, next.sec))
        {
            throw FlexException(FERR_READING_TRKSEC,
                                next.trk, next.sec, fp.GetPath());
        }

        for (i = 0; i < 10; i++)
        {
            // look for the next free directory entry
            pde = &ds.dir_entry[i];

            if (pde->filename[0] == DE_EMPTY || pde->filename[0] == DE_DELETED)
            {
                int records;

                records = (entry.GetSize() / param.byte_p_sector) +
                          (entry.IsRandom() ? 2 : 0);
                memset(pde->filename, 0, FLEX_BASEFILENAME_LENGTH);
                strncpy(pde->filename, entry.GetFileName().c_str(),
                        FLEX_BASEFILENAME_LENGTH);
                memset(pde->file_ext, 0, FLEX_FILEEXT_LENGTH);
                strncpy(pde->file_ext, entry.GetFileExt().c_str(),
                        FLEX_FILEEXT_LENGTH);
                pde->file_attr = entry.GetAttributes();
                pde->reserved1 = 0;
                entry.GetStartTrkSec(tmp1, tmp2);
                pde->start.trk = static_cast<Byte>(tmp1);
                pde->start.sec = static_cast<Byte>(tmp2);
                entry.GetEndTrkSec(tmp1, tmp2);
                pde->end.trk = static_cast<Byte>(tmp1);
                pde->end.sec = static_cast<Byte>(tmp2);
                setValueBigEndian<Word>(&pde->records[0], records);
                pde->sector_map = (entry.IsRandom() ? IS_RANDOM_FILE : 0x00);
                pde->reserved2 = 0;
                date = entry.GetDate();
                pde->day = static_cast<Byte>(date.GetDay());
                pde->month = static_cast<Byte>(date.GetMonth());
                pde->year = static_cast<Byte>(date.GetYear() % 100);

                if (!WriteSector((Byte *)&ds, next.trk, next.sec))
                {
                    throw FlexException(FERR_WRITING_TRKSEC,
                                        next.trk, next.sec, fp.GetPath());
                }

                return true;
            }
        }

        next = ds.next;
    }

    throw FlexException(FERR_DIRECTORY_FULL);
    return true; // satisfy compiler
}


/****************************************/
/* low level routines                   */
/****************************************/

int FlexFileContainer::ByteOffset(const int trk, const int sec) const
{
    int byteOffs;

    byteOffs = param.offset;

    if (trk > 0)
    {
        byteOffs += param.byte_p_track0;
        byteOffs += param.byte_p_track * (trk - 1);
    }

    byteOffs += param.byte_p_sector * (sec - 1);
    return byteOffs;
}

// low level routine to read a single sector
// should be used with care
// Does not throw any exception !
// returns false on failure
bool FlexFileContainer::ReadSector(Byte *pbuffer, int trk, int sec) const
{
    int pos;

    if (fp == nullptr)
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    pos = ByteOffset(trk, sec);

    if (pos < 0)
    {
        return false;
    }

    if (fseek(fp, pos, SEEK_SET))
    {
        return false;
    }

    if (fread(pbuffer, param.byte_p_sector, 1, fp) != 1)
    {
        return false;
    }

    return true;
}

// low level routine to write a single sector
// should be used with care
// Does not throw any exception !
// returns false on failure
bool FlexFileContainer::WriteSector(const Byte *pbuffer, int trk, int sec)
{
    int pos;

    if (fp == nullptr)
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    pos = ByteOffset(trk, sec);

    if (pos < 0)
    {
        return false;
    }

    if (fseek(fp, pos, SEEK_SET))
    {
        return false;
    }

    if (fwrite(pbuffer, param.byte_p_sector, 1, fp) != 1)
    {
        return false;
    }

    return true;
}

void FlexFileContainer::Initialize_for_flx_format(
    s_floppy    *pfloppy,
    s_flex_header   *pheader,
    bool        wp)
{
    pfloppy->offset        = sizeof(struct s_flex_header);
    pfloppy->write_protect =
        (wp || pheader->write_protect) ? 0x40 : 0;
    pfloppy->max_sector    = pheader->sectors * pheader->sides;
    pfloppy->max_sector0   = pheader->sectors0 * pheader->sides0;
    pfloppy->max_track     = pheader->tracks - 1;
    pfloppy->byte_p_sector = 128 << pheader->sizecode;
    pfloppy->byte_p_track0 =
        pheader->sides0 * pheader->sectors0 * pfloppy->byte_p_sector;
    pfloppy->byte_p_track  =
        pheader->sides * pheader->sectors * pfloppy->byte_p_sector;
    pfloppy->type          = TYPE_CONTAINER | TYPE_FLX_CONTAINER;

} // initialize_for_flx_format

void FlexFileContainer::Initialize_for_dsk_format(
    s_floppy    *pfloppy,
    s_formats   *pformat,
    bool        wp)
{
    pfloppy->offset        = 0;
    pfloppy->write_protect = wp ? 1 : 0;
    pfloppy->max_sector    = pformat->sectors;
    pfloppy->max_sector0   = pformat->sectors;
    pfloppy->max_track     = pformat->tracks - 1;
    pfloppy->byte_p_sector = SECTOR_SIZE;
    pfloppy->byte_p_track0 = pformat->sectors * SECTOR_SIZE;
    pfloppy->byte_p_track  = pformat->sectors * SECTOR_SIZE;
    pfloppy->type          = TYPE_CONTAINER | TYPE_DSK_CONTAINER;
} // initialize_for_dsk_format

void FlexFileContainer::Create_boot_sectors(Byte sec_buf[], Byte sec_buf2[])
{
    // Read boot sector(s) if present from file.
    BFilePtr boot(bootSectorFile.c_str(), "rb");

    if (boot == nullptr || fread(sec_buf, SECTOR_SIZE, 1, boot) != 1)
    {
        // No boot sector or read error.
        // Instead jump to monitor program warm start entry point.
        memset(sec_buf, 0, SECTOR_SIZE);
        sec_buf[0] = 0x7E; // JMP $F02D
        sec_buf[1] = 0xF0;
        sec_buf[2] = 0x2D;
    }
    if (boot != nullptr && fread(sec_buf2, SECTOR_SIZE, 1, boot) != 1)
    {
        memset(sec_buf2, 0, SECTOR_SIZE);
    }
} // Create_boot_sectors

void FlexFileContainer::Create_sys_info_sector(Byte sec_buf[], const char *name,
        struct s_formats *fmt)
{
    int     i, start, free;
    time_t      time_now;
    struct tm   *lt;

    memset(sec_buf, 0, SECTOR_SIZE);

    auto &sis = *reinterpret_cast<s_sys_info_sector *>(sec_buf);

    for (i = 0; i < 8; i++)
    {
        if (*(name + i) == '.' || *(name + i) == '\0')
        {
            break;
        }

        sis.sir.disk_name[i] = *(name + i);
    } // for

    start           = fmt->dir_sectors + 4;
    free            = (fmt->sectors * fmt->tracks) - start;
    time_now        = time(nullptr);
    lt          = localtime(&time_now);
    setValueBigEndian<Word>(&sis.sir.disk_number[0], 1U);
    sis.sir.fc_start.trk = static_cast<Byte>(start / fmt->sectors);
    sis.sir.fc_start.sec = static_cast<Byte>((start % fmt->sectors) + 1);
    sis.sir.fc_end.trk = static_cast<Byte>(fmt->tracks - 1);
    sis.sir.fc_end.sec = static_cast<Byte>(fmt->sectors);
    setValueBigEndian<Word>(&sis.sir.free[0], free);
    sis.sir.month = static_cast<Byte>(lt->tm_mon + 1);
    sis.sir.day = static_cast<Byte>(lt->tm_mday);
    sis.sir.year = static_cast<Byte>(lt->tm_year);
    sis.sir.last.trk = static_cast<Byte>(fmt->tracks - 1);
    sis.sir.last.sec = static_cast<Byte>(fmt->sectors);
} // create_sys_info_sectors

// on success return true
bool FlexFileContainer::Write_dir_sectors(FILE *fp, struct s_formats *fmt)
{
    Byte    sec_buf[SECTOR_SIZE];
    int     i;

    memset(sec_buf, 0, sizeof(sec_buf));

    for (i = 0; i < fmt->dir_sectors; i++)
    {
        sec_buf[0] = 0;
        sec_buf[1] = 0;

        if (i < fmt->dir_sectors - 1)
        {
            sec_buf[0] = static_cast<Byte>((i + 5) / fmt->sectors);
            sec_buf[1] = static_cast<Byte>(((i + 5) % fmt->sectors) + 1);
        }

        if (fwrite(sec_buf, sizeof(sec_buf), 1, fp) != 1)
        {
            return false;
        }
    }

    return true;
} // write_dir_sectors

// on success return true
bool FlexFileContainer::Write_sectors(FILE *fp, struct s_formats *fmt)
{
    Byte    sec_buf[SECTOR_SIZE];
    int     i;

    memset(sec_buf, 0, sizeof(sec_buf));

    for (i = fmt->dir_sectors + 5;
         i <= fmt->sectors * fmt->tracks; i++)
    {
        sec_buf[0] = static_cast<Byte>(i / fmt->sectors);
        sec_buf[1] = static_cast<Byte>((i % fmt->sectors) + 1);

        // use for tests to correctly save random files:
        // (the link always jumps over one sector)
        //      sec_buf[0] = (i+1) / fmt->sectors;
        //      sec_buf[1] = ((i+1) % fmt->sectors) + 1;
        if (i == fmt->sectors * fmt->tracks)
        {
            sec_buf[0] = sec_buf[1] = 0;
        }

        if (fwrite(sec_buf, sizeof(sec_buf), 1, fp) != 1)
        {
            return false;
        }
    }

    return true;
} // write_sectors

void FlexFileContainer::Create_format_table(int trk, int sec,
        struct s_formats *pformat)
{
    SDWord tmp;

    if (trk < 2)
    {
        trk = 2;
    }
    else if (trk > 255)
    {
        trk = 255;
    }

    if (sec < 5)
    {
        sec = 5;
    }
    else if (sec > 255)
    {
        sec = 255;
    }

    pformat->tracks = static_cast<Word>(trk);
    pformat->sectors = static_cast<Word>(sec);

    pformat->size = pformat->tracks * pformat->sectors * SECTOR_SIZE;
    tmp = pformat->size / DIRSECTOR_PER_KB;
    // calculate number of directory sectors
    // at least track 0 only contains directory sectors
    pformat->dir_sectors = static_cast<Word>((tmp < sec - 4) ? sec - 4 : tmp);
} // create_format_table

// return != 0 on success
// format FLX or DSK format. FLX format always with sector_size 256
// and same nr of sectors on track 0 and != 0.
// type:
//  use TYPE_DSK_CONTAINER for DSK format
//  use TYPE_FLX_CONTAINER for FLX format

void FlexFileContainer::Format_disk(
    int trk,
    int sec,
    const char *disk_dir,
    const char *name,
    int  type /* = TYPE_DSK_CONTAINER */)
{
    std::string path;
    struct s_formats format;
    struct s_flex_header hdr;
    int     err = 0;

    if (disk_dir == nullptr ||
        name == nullptr || strlen(name) == 0 ||
        trk < 2 || sec < 6 || trk > 255 || sec > 255)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    Create_format_table(trk, sec, &format);

    path = disk_dir;

    if (path.size() > 0 && path[path.size() - 1] != PATHSEPARATOR)
    {
        path +=PATHSEPARATORSTRING;
    }

    path += name;

    BFilePtr fp(path.c_str(), "wb");

    if (fp != nullptr)
    {
        Byte sector_buffer[SECTOR_SIZE];

        if (type == TYPE_FLX_CONTAINER)
        {
            hdr.initialize(SECTOR_SIZE, format.tracks, format.sectors,
                           format.sectors, 1);

            if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
            {
                err = 1;
            }
        }

        {
            Byte sector_buffer2[SECTOR_SIZE];

            Create_boot_sectors(sector_buffer, sector_buffer2);

            if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
            {
                err = 1;
            }

            if (fwrite(sector_buffer2, sizeof(sector_buffer2), 1, fp) != 1)
            {
                err = 1;
            }
        }

        Create_sys_info_sector(sector_buffer, name, &format);

        if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
        {
            err = 1;
        }

        if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
        {
            err = 1;
        }

        if (!Write_dir_sectors(fp, &format))
        {
            err = 1;
        }

        if (!Write_sectors(fp, &format))
        {
            err = 1;
        }
    }
    else
    {
        err = 1;
    }

    if (err)
    {
        throw FlexException(FERR_UNABLE_TO_FORMAT, name);
    }
} // format_disk

