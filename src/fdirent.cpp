/*
    fdirent.cpp


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
#include "bdate.h"
#include "bstring.h"
#include "fdirent.h"
#include <stdio.h>

char *FlexDirEntry::fileDescription[] = {
	"BIN", "Binary file",
	"TXT", "Text file",
	"CMD", "Executable file",
	"BAS", "Basic file",
	"SYS", "System file",
	"BAK", "Backup file",
	"BAC", "Backup file",
	"DAT", "Data file",
	NULL
};


FlexDirEntry::FlexDirEntry() :
	size(0),
	attributes(0),
	sectorMap(0),
	startTrk(-1),
	startSec(0),
	endTrk(0),
	endSec(0),
	status(0)
{
} // FlexDirEntry

FlexDirEntry::~FlexDirEntry(void)
{
}

FlexDirEntry::FlexDirEntry(const FlexDirEntry& de)
{
	CopyFrom(de);
}

void FlexDirEntry::CopyFrom(const FlexDirEntry& de)
{
	date		= de.date;
	fileName	= de.fileName;
	size		= de.size;
	attributes	= de.attributes;
	startTrk	= de.startTrk;
	startSec	= de.startSec;
	endTrk		= de.endTrk;
	endSec		= de.endSec;
	sectorMap	= de.sectorMap;
}
	

const BDate& FlexDirEntry::GetDate(void) const
{
	return date;
}

void FlexDirEntry::SetDate(const BDate& d)
{
	date = d;
}

void FlexDirEntry::SetDate(int d, int m, int y)
{
	date.SetDate(d, m, y);
}


void FlexDirEntry::SetTotalFileName(const char *s)
{
	fileName = s;
	fileName.upcase();
}

const char *FlexDirEntry::GetFileName(void) const
{
	int i;
	char *p;
	static char name[FLEX_BASEFILENAME_LENGTH+1];

	p = strchr(fileName, '.');
	if (p == NULL)
		i = FLEX_BASEFILENAME_LENGTH;
	else
		i = p - fileName.chars();
	strncpy(name, fileName, i);
	name[i] = '\0';
	return name;
}

const char *FlexDirEntry::GetFileExt(void)
{
	static char ext[FLEX_FILEEXT_LENGTH+1];
	char *p;

	ext[0] = '\0';
	p = strchr(fileName, '.');
	if (p == NULL)
		return ext;
	strncpy(ext, p+1, FLEX_FILEEXT_LENGTH);
	ext[FLEX_FILEEXT_LENGTH] = '\0';
	return ext;
}

const BString FlexDirEntry::GetFileDescription(void)
{
	BString		description;
	char		**pExt;
	const char	*extension;

	pExt = (char **)fileDescription;
	extension = GetFileExt();
	while (*pExt != NULL) {
		if (!strcmp(*pExt, extension)) {
			description = *(++pExt);
			return description;
		}
		pExt += 2;
	}
	description = extension;
	description += " file";
	return description;
}

const BString& FlexDirEntry::GetTotalFileName(void) const
{
	return fileName;
}

void FlexDirEntry::SetStartTrkSec(int t, int s) {
	startTrk = t;
	startSec = s;
}

void FlexDirEntry::GetStartTrkSec(int *t, int *s) {
	*t = startTrk;
	*s = startSec;
}

void FlexDirEntry::SetEndTrkSec(int t, int s) {
	endTrk = t;
	endSec = s;
}

void FlexDirEntry::GetEndTrkSec(int *t, int *s) {
	*t = endTrk;
	*s = endSec;
}

const BString FlexDirEntry::GetAttributesString(void)
{
	BString str;

	if (attributes & FLX_READONLY)	str += "W";
	if (attributes & FLX_NODELETE)	str += "D";
	if (attributes & FLX_NOREAD)	str += "R";
	if (attributes & FLX_NOCAT)		str += "C";
	return str;
}

void FlexDirEntry::SetAttributes(int setMask, int clearMask)
{
	attributes = (attributes & ~clearMask) | setMask;
}
