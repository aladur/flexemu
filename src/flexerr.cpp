/*
    flexerr.cpp


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
#include <exception>
#include <stdio.h>
#include "flexerr.h"


FlexException::FlexException() throw()
{
	errorCode = 0;
	strcpy(errorString, errString[errorCode]);
}

FlexException::~FlexException() throw()
{
}

const char *FlexException::what() const throw()
{
	return errorString;
}

void FlexException::setString(int ec)
{
	errorCode = ec;
	strcpy(errorString, errString[ec]);
}

void FlexException::setString(int ec, int ip1)
{
	errorCode = ec;
	sprintf(errorString, errString[ec], ip1);
}

void FlexException::setString(int ec, const char *sp1)
{
	errorCode = ec;
	sprintf(errorString, errString[ec], sp1);
}

void FlexException::setString(int ec, const char *sp1, const char *sp2)
{
	errorCode = ec;
	sprintf(errorString, errString[ec], sp1, sp2);
}

void FlexException::setString(int ec, int ip1, int ip2, const char *sp1)
{
	errorCode = ec;
	sprintf(errorString, errString[ec], ip1, ip2, sp1);
}

#ifdef WIN32
void FlexException::setWindowsError(int lastError, const char* sp1)
{
	LPVOID lpMsgBuf;

	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, lastError, 0, (LPTSTR) &lpMsgBuf, 0, NULL))
		sprintf(errorString, "An unspecified Windows error occured (#%d)", lastError);
	if (sp1 != NULL)
		sprintf(errorString, "%s%s", (char *)lpMsgBuf, sp1);
	else
		sprintf(errorString, "%s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
#endif

const char *FlexException::errString[] = {
	"No Error",
	"Unable to open %s",
	"%s is no file container",
	"No container opened",
	"No file opened",
	"Unable to format %s",
	"Invalid container format #%d",
	"Error reading from %s",
	"Error writing to %s",
	"Directory already opened",
	"No directory opened",
	"File already opened",
	"No free file handle available",
	"File %s already exists",
	"Invalid file handle #%d",
	"Invalid open mode \"%s\"",
	"Directory full",
	"Error reading trk/sec %02d/%02d in %s",
	"Error writing trk/sec %02d/%02d in %s",
	"No file \"%s\" (container %s)",
	"Record map of %s is full (container %s)",
	"Container %s full when writing %s",
	"Unable to create %s",
	"Unable to rename %s (container %s)",
	"Unable to remove %s (container %s)",
	"Error reading disk space (container %s)",
	"Unable to copy %s on itself",
	"Wrong parameter",
	"Error creating process (%s %s)",
	"Error reading FLEX binary format",
	"Error creating temporary file %s",
	"Container %s is read-only"
};

