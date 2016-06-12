/*
    bin2s19.c

    convert a binary file to Motorola S-Record format
    Copyright (C) 2000  W. Schwotzer

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int writeS19(char *srcFileName, int startAddress, int max)
{
	char	*tgtFileName;
	FILE 	*fps, *fpt;
	int	i;
	unsigned int	count;
	int	byte;
	int	checkSum;

	/* first open source file */
	fps = fopen(srcFileName, "r");
	if (!fps) {
		fprintf(stderr, "Unable to open %s\n", srcFileName);
		return 0;
	}
	/* replace file extension with "s19" */
	tgtFileName = malloc(strlen(srcFileName) + 5);
	strcpy(tgtFileName, srcFileName);
	i = strlen(tgtFileName) - 1;
	while (i) {
		if (tgtFileName[i] == '.') {
			tgtFileName[i] = '\0';
			strcat(tgtFileName, ".s19");
			break;
		}
		i--;
	}

	/* open target file */
	fpt = fopen(tgtFileName, "w");
	if (!fpt) {
		fclose(fps);
		fprintf(stderr, "Unable to open %s\n", tgtFileName);
		free(tgtFileName);
		tgtFileName = NULL;
		return 0;
	}

	/* read from source, write to target */
	i = 0;
	checkSum = 0;
	count = 0;
	while ((byte = fgetc(fps)) != EOF) {
		if (!count) {
			if (byte != 0x02) {
				free(tgtFileName);
				fclose(fps); fclose(fpt);
				return 1;
			}
			fgetc(fps); fgetc(fps); /* ignore address */
			count = fgetc(fps);
			byte = fgetc(fps);
		}
		checkSum += byte;
		if (!(i % max))
			fprintf(fpt, "S1%02X%04X", max+3, startAddress + i);
		fprintf(fpt, "%02X", byte);
		i++;
		count--;
		if (!(i % max)) {
			fprintf(fpt, "%02X\n", checkSum & 0xFF);
			checkSum = 0;
		}
	}
	fclose(fps); fclose(fpt);
	return 1;
}

int main(int argc, char **argv)
{
	char    *optstr = "f:s:hn:";
	int     startAddr, max;
	char *fileName;

	/*optind = 1;
	opterr = 0;*/
	fileName = NULL;
	startAddr = 0;
	max = 32;
	while (1) {
		int result = getopt(argc, argv, optstr);
		if (result == -1)
			break;
		switch(result) {
			case 'f': fileName = optarg;
				  break;
			case 'n': sscanf(optarg, "%d", &max);
				  break;
			case 's': sscanf(optarg, "%x", (unsigned int *)&startAddr);
				  break;
			case 'h': fprintf(stdout, "syntax: %s -f<filename> [-sXXXX]", argv[0]);
				  exit(0);
		}  /* switch */
	} /* while */
	if (fileName == NULL) {
		fprintf(stderr, "Please specify a filename with -f<filename>\n");
		exit(1);
	}
	writeS19(fileName, startAddr, max);
	return 0;
}

