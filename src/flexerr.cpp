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

#ifdef _
#undef _
#endif

#define _(p) p

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

const char *FlexException::wwhat() const throw()
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
		sprintf(errorString, errString[FERR_UNSPEC_WINDOWS_ERROR], lastError);
	if (sp1 != NULL)
		sprintf(errorString, "%s%s", (char *)lpMsgBuf, sp1);
	else
		sprintf(errorString, "%s", (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
#endif

const char *FlexException::errString[] = {
	_("No Error"),
	_("Unable to open %s"),
	_("%s is no file container"),
	_("No container opened"),
	_("No file opened"),
	_("Unable to format %s"),
	_("Invalid container format #%d"),
	_("Error reading from %s"),
	_("Error writing to %s"),
	_("Directory already opened"),
	_("No directory opened"),
	_("File already opened"),
	_("No free file handle available"),
	_("File %s already exists"),
	_("Invalid file handle #%d"),
	_("Invalid open mode \"%s\""),
	_("Directory full"),
	_("Error reading trk/sec %02d/%02d in %s"),
	_("Error writing trk/sec %02d/%02d in %s"),
	_("No file \"%s\" (container %s)"),
	_("Record map of %s is full (container %s)"),
	_("Container %s full when writing %s"),
	_("Unable to create %s"),
	_("Unable to rename %s (container %s)"),
	_("Unable to remove %s (container %s)"),
	_("Error reading disk space (container %s)"),
	_("Unable to copy %s on itself"),
	_("Wrong parameter"),
	_("Error creating process (%s %s)"),
	_("Error reading FLEX binary format"),
	_("Error creating temporary file %s"),
	_("Container %s is read-only"),
	_("An unspecified Windows error occured (#%d)")
};

