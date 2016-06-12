/*
    flexerr.h


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

#ifndef __flexerr_h__
#define __flexerr_h__

#define MAX_ERR_STRINGLENGTH 1024


class wxString;

#define FERR_NOERROR			(0)
#define FERR_UNABLE_TO_OPEN		(1)
#define FERR_IS_NO_FILECONTAINER	(2)
#define FERR_NO_CONTAINER_OPEN		(3)
#define FERR_NO_FILE_OPENED		(4)
#define FERR_UNABLE_TO_FORMAT		(5)
#define FERR_INVALID_FORMAT		(6)
#define FERR_READING_FROM		(7)
#define FERR_WRITING_TO			(8)
#define FERR_DIRECTORY_ALREADY_OPENED	(9)
#define FERR_DIRECTORY_NOT_OPENED	(10)
#define FERR_FILE_ALREADY_OPENED	(11)
#define FERR_NO_FILEHANDLE_AVAILABLE	(12)
#define FERR_FILE_ALREADY_EXISTS	(13)
#define FERR_INVALID_FILEHANDLE		(14)
#define FERR_INVALID_MODE		(15)
#define FERR_DIRECTORY_FULL		(16)
#define FERR_READING_TRKSEC		(17)
#define FERR_WRITING_TRKSEC		(18)
#define FERR_NO_FILE_IN_CONTAINER	(19)
#define FERR_RECORDMAP_FULL		(20)
#define FERR_DISK_FULL_WRITING		(21)
#define FERR_UNABLE_TO_CREATE		(22)
#define FERR_RENAME_FILE		(23)
#define FERR_REMOVE_FILE		(24)
#define FERR_READING_DISKSPACE		(25)
#define FERR_COPY_ON_ITSELF		(26)
#define FERR_WRONG_PARAMETER		(27)
#define FERR_CREATE_PROCESS		(28)
#define FERR_WRONG_FLEX_BIN_FORMAT	(29)
#define FERR_CREATE_TEMP_FILE		(30)
#define FERR_CONTAINER_IS_READONLY	(31)

class FlexException
{
private:

	int	 errorCode;
	char errorString[MAX_ERR_STRINGLENGTH];
        static const char *errString[];

public:

	FlexException() throw();
	virtual ~FlexException() throw();

	virtual const char *what() const throw();

	int	GetErrorCode(void) const { return errorCode; };
	void	setString(int ec);
	void	setString(int ec, int ip1);
	void	setString(int ec, const char *sp1);
	void	setString(int ec, const char *sp1, const char *sp2);
	void	setString(int ec, int ip1, int ip2, const char *sp1);
#ifdef WIN32
	void	setWindowsError(int lastError, const char *sp1 = NULL);
#endif
};


#endif  // #ifndef __flexerr_h__
