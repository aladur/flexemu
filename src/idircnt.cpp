/*
    idircnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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

#ifndef __idircnt_cpp__
#define __idircnt_cpp__

#include <misc1.h>
#ifdef _MSC_VER
#include <io.h>         // needed for access
#include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "filecont.h"
#include "dircont.h"
#include "idircnt.h"


DirectoryContainerIteratorImp::DirectoryContainerIteratorImp(
		DirectoryContainer *aBase)
 : base(aBase), dirHdl(NULL)
{
}

DirectoryContainerIteratorImp::~DirectoryContainerIteratorImp()
{
        if (dirHdl != NULL) {
#ifdef WIN32
                FindClose(dirHdl);
#endif
#ifdef UNIX
                closedir(dirHdl);
#endif
                dirHdl = NULL;
        }
        base = NULL;
}

bool DirectoryContainerIteratorImp::operator==(const FileContainerIf *src) const
{
        return (base == NULL && src == NULL) ||
                (((FileContainerIf *)base == src) && (dirHdl == NULL));
}

void DirectoryContainerIteratorImp::AtEnd()
{
	base = NULL;
}

bool DirectoryContainerIteratorImp::NextDirEntry(const char *filePattern)
{
        BString         str, fileName;
        int                     attribs, sectorMap;
        bool                    isValid;
#ifdef WIN32
        WIN32_FIND_DATA         findData;
        SYSTEMTIME              systemTime;
#endif
#ifdef UNIX
        struct dirent           *findData = NULL;
        struct stat             sbuf;
#endif

        dirEntry.SetEmpty();
        // repeat until a valid directory entry found
#ifdef WIN32
        str = base->GetPath();
        str += PATHSEPARATOR;
        str += "*.*";
        do {
                isValid = false;
                if (dirHdl == NULL) {
                        dirHdl = FindFirstFile(str, &findData);
                        if (dirHdl != INVALID_HANDLE_VALUE)
                                isValid = true;
                        else
                                dirHdl = NULL;
                } else {
                        if (FindNextFile(dirHdl, &findData))
			{
                                isValid = true;
                		if (strlen(findData.cFileName) > 12)
                        		fileName = findData.cAlternateFileName;
                		else
                        		fileName = findData.cFileName;
			}
                }
        } while (isValid &&
                   ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                        (findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) ||
                        (findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ||
                        !stricmp(fileName, RANDOM_FILE_LIST) ||
                        findData.nFileSizeHigh != 0 ||
			!fileName.multimatches(filePattern, ';', true)));
        if (isValid) {
        // ok, found a valid directory entry
                attribs = 0;
                sectorMap = 0;
                dirEntry.SetTotalFileName(fileName);
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                        sectorMap = 2;
                // CDFS support:
                if (base->IsRandomFile(base->GetPath(), fileName))
                        sectorMap = 2;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        attribs |= WRITE_PROTECT;
                dirEntry.SetSize(findData.nFileSizeLow);
                FileTimeToSystemTime(&findData.ftLastWriteTime, &systemTime);
                dirEntry.SetDate(systemTime.wDay, systemTime.wMonth,
			systemTime.wYear);
                dirEntry.SetAttributes(attribs);
                dirEntry.SetSectorMap(sectorMap);
                dirEntry.SetStartTrkSec(0, 0);
                dirEntry.SetEndTrkSec(0, 0);
                dirEntry.ClearEmpty();
        }
#endif
#ifdef UNIX
        // unfinished
        str = base->GetPath();
        do {
                isValid = false;
                if (dirHdl == NULL) {
                        if ((dirHdl = opendir(str)) != NULL)
                                isValid = true;
                }
                if (dirHdl != NULL && (findData = readdir(dirHdl)) != NULL)
		{
                        isValid = true;
			fileName = findData->d_name;
		}
        } while (isValid &&
                (stat(str + PATHSEPARATORSTRING + findData->d_name, &sbuf) ||
                 !base->IsFlexFilename(findData->d_name) ||
                 !S_ISREG(sbuf.st_mode) ||
                 sbuf.st_size <= 0 ||
                 !fileName.multimatches(filePattern, ';', true)));
        if (isValid) {
        	struct tm *lt;

        // ok, found a valid directory entry
                attribs = 0;
                sectorMap = 0;
                if (base->IsWriteProtected()) {
                        // CDFS-Support: look for file name in file 'random'
                        if (base->IsRandomFile(
				base->GetPath(), findData->d_name))
                                sectorMap = 2;
                } else {
                        if (sbuf.st_mode & S_IXUSR)
                                sectorMap = 2;
                }
                if (!(sbuf.st_mode & S_IWUSR))
                        attribs |= WRITE_PROTECT;
                dirEntry.SetTotalFileName(findData->d_name);
                dirEntry.SetSize(sbuf.st_size);
                lt = localtime(&(sbuf.st_mtime));
                dirEntry.SetDate(lt->tm_mday, lt->tm_mon+1, lt->tm_year+1900);
                dirEntry.SetAttributes(attribs);
                dirEntry.SetSectorMap(sectorMap);
                dirEntry.SetStartTrkSec(0, 0);
                dirEntry.SetEndTrkSec(0, 0);
                dirEntry.ClearEmpty();
        }
#endif
        return !dirEntry.IsEmpty();
}

// deletes the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::DeleteCurrent(void)
{
        BString filePath;

	if (dirEntry.IsEmpty())
		return false;

        filePath = dirEntry.GetTotalFileName();
        filePath.downcase();
        filePath = base->GetPath() + PATHSEPARATOR + filePath;
#ifdef UNIX
        if (remove(filePath)) {
		FlexException ex;
                if (errno == ENOENT)
                    ex.setString(FERR_NO_FILE_IN_CONTAINER,
						dirEntry.GetTotalFileName(), base->GetPath());
				else
					ex.setString(FERR_REMOVE_FILE,
						dirEntry.GetTotalFileName(), base->GetPath());

				throw ex;
        }
#endif
#ifdef WIN32
		// evtl. remove read-only attribute
		// to be able to delete it
		DWORD attributes = GetFileAttributes(filePath);
		if (attributes & FILE_ATTRIBUTE_READONLY)
			SetFileAttributes(filePath, attributes & ~FILE_ATTRIBUTE_READONLY);

		if (!DeleteFile(filePath))
		{
			FlexException ex;
			DWORD lastError = GetLastError();

            if (lastError == ERROR_FILE_NOT_FOUND)
					ex.setString(FERR_NO_FILE_IN_CONTAINER,
					dirEntry.GetTotalFileName(), base->GetPath());
			else
					ex.setString(FERR_REMOVE_FILE,
					dirEntry.GetTotalFileName(), base->GetPath());

            throw ex;
		}
#endif
        return true;
}

// Renames the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::RenameCurrent(const char *newName)
{
	if (dirEntry.IsEmpty())
		return false;

        BString s(dirEntry.GetTotalFileName());
        BString d(newName);
	FlexDirEntry de;
#ifdef UINX
        s.downcase();
        d.downcase();
#endif
        // prevent overwriting of an existing file
        if (base->FindFile(d, de)) {
		FlexException ex;
                ex.setString(FERR_FILE_ALREADY_EXISTS, newName);
                throw ex;
        }
        if (s == d)
                return true;

        s = base->GetPath() + PATHSEPARATORSTRING + s;
        d = base->GetPath() + PATHSEPARATORSTRING + d;
        if (rename(s, d)) {
		FlexException ex;
                // Unfinished
                if (errno == EEXIST)
                        ex.setString(FERR_FILE_ALREADY_EXISTS, newName);
                if (errno == EACCES)
                        ex.setString(FERR_RENAME_FILE,
				dirEntry.GetTotalFileName(), base->GetPath());
                if (errno == ENOENT)
                        ex.setString(FERR_NO_FILE_IN_CONTAINER,
				dirEntry.GetTotalFileName(), base->GetPath());
                throw ex;
        }
        return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::SetDateCurrent(const BDate& date)
{
        struct stat    sbuf;
        struct utimbuf timebuf;
        struct tm      file_time;
        BString        filePath;

	if (dirEntry.IsEmpty())
		return false;

        filePath = dirEntry.GetTotalFileName();
        filePath.downcase();
        filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;
        if (stat(filePath, &sbuf) >= 0) {
                timebuf.actime = sbuf.st_atime;
                file_time.tm_sec   = 0;
                file_time.tm_min   = 0;
                file_time.tm_hour  = 12;
                file_time.tm_mon   = date.GetMonth()-1;
                file_time.tm_mday  = date.GetDay();
                file_time.tm_year  = date.GetYear() - 1900;
                file_time.tm_isdst = 0;
                timebuf.modtime    = mktime(&file_time);
                if (timebuf.modtime >= 0 && utime(filePath, &timebuf) >= 0)
                        return true;
                else
                        return false;
        } // if
        return false;
}

// set the date in the actual selected directory entry
// Only valid if the iterator has a valid directory entry
// Only the WRITE_PROTECT flag is supported
bool DirectoryContainerIteratorImp::SetAttributesCurrent(int attributes)
{
        BString filePath;

	if (dirEntry.IsEmpty())
		return false;

#ifdef WIN32
	filePath = base->GetPath() + PATHSEPARATORSTRING +
			dirEntry.GetTotalFileName();
	DWORD attrs = GetFileAttributes(filePath);
	if (attributes & WRITE_PROTECT)
		attrs |= FILE_ATTRIBUTE_READONLY;
	else
		attrs &= ~FILE_ATTRIBUTE_READONLY;
	SetFileAttributes(filePath, attrs);
#endif
#ifdef UNIX
	struct stat sbuf;

	filePath = dirEntry.GetTotalFileName();
	filePath.downcase();
	filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;
	if (!stat(filePath, &sbuf)) {
		if (attributes & WRITE_PROTECT)
			chmod(filePath, sbuf.st_mode | S_IWUSR);
		else
			chmod(filePath, sbuf.st_mode & ~S_IWUSR);
	}
#endif
	return true;
}

#endif // __idircnt_h__

