/* rcvflex.c

program to transfer a total disc image through a serial port from
FLEX to UNIX

*/


#include <stdio.h>

#if defined(__linux__) || \
    defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#include <ctype.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#define __fromflex__
#include "typedefs.h"
#include "../filecnts.h"

#define DEFAULT_DEVICE	"/dev/cua2"
#define BYTES_P_BLOCK	16	


struct s_flex_header 	flh;	/* file header				*/
FILE 			*fp;	/* file pointer to target file		*/
struct termio 		oldtty, ntty;
int			fd;	/* file descriptor for serial device	*/
char			*device; /* pointer to serial device		*/

void do_exit(int exit_code)
{
	if (fp)
		fclose(fp);
	ioctl(fd, TCSETA, &oldtty);
	close(fd);
	exit(exit_code);
} /* do_exit */

#ifdef sun
	void sigint(int param, ...)
#else
	void sigint(int param)
#endif
{
        (void)param;
	do_exit(0);
} /* sigint */


int main(int argc, char *argv[])
{
	int 		i, j, errcnt, si, se, tr, blk;
	char 		filename[13];
	unsigned char	buffer[2], blk_buffer[BYTES_P_BLOCK + 4];
	char		*commit = "YYYY", *err = "NNNN";
	unsigned short 	chksum;
	long unsigned int m;
        ssize_t         count = 0;

	if (argc > 2 || (argc == 2 &&
		*argv[1] == '-' && tolower(*(argv[1]+1) == 'h'))) {
		fprintf(stdout, "syntax: rcvflex [serial_device]\n");
		fprintf(stdout, "   %s is the default device\n", DEFAULT_DEVICE);
		exit (1);
	}
	if (argc == 2)
		device = argv[1];
	else
		device = DEFAULT_DEVICE;

	fp = NULL;
	fd = open(device, O_RDWR | O_NOCTTY);

	if (ioctl(fd, TCGETA, &oldtty) == -1) {
		fprintf(stderr, "%s is wrong device\n", device);
		exit(1);
	}
	memcpy(&ntty, &oldtty, sizeof(struct termio));
	signal(SIGINT, sigint);
	ntty.c_iflag = 0;
	ntty.c_oflag = 0;
	ntty.c_cflag = (tcflag_t)(CLOCAL | CREAD | B19200 | CS8);
	ntty.c_lflag = 0;
	ntty.c_cc[VMIN] = 0;
	ntty.c_cc[VTIME] = 50;
	if (ioctl(fd, TCSETAF, &ntty) == -1) {
		fprintf(stderr, "can't set serial line %s\n", device);
		do_exit(1);
	}

	tcflush(fd, TCIOFLUSH); /* flush input/output */
	/* syncronize */
	do {	
		count = read(fd, &buffer, 1);
		if (buffer[0] != 'S')
			count = write(fd, "A", 1); /* reply with 'A' */
	} while(buffer[0] != 'S');
	count = write(fd, "T", 1); /* send one time 'T' */

 	/* read fileheader */
 	chksum = 0;
	do {
		flh.magic_number = 0;
		for (i = 1; i <= 4; i++) {
			count = read(fd, &buffer, 1);
			chksum = (chksum + buffer[0]) % 256;
			flh.magic_number = (flh.magic_number << 8) | buffer[0]; 
		}
		count = read(fd, &flh.write_protect, 1);
		count = read(fd, &flh.sizecode, 1);
		count = read(fd, &flh.sides0, 1);
		count = read(fd, &flh.sectors0, 1);
		count = read(fd, &flh.sides, 1);
		count = read(fd, &flh.sectors, 1);
		count = read(fd, &flh.tracks, 1);
		/*sectorsize = (128 << flh.sizecode);
		total = (flh.sides0 * flh.sectors0 
			+ flh.sides * flh.sectors *
				(flh.tracks -1))
			* sectorsize;*/
		chksum = (chksum + flh.write_protect + flh.sizecode +
			 flh.sides0 + flh.sectors0 +
			 flh.sides + flh.sectors +
			 flh.tracks) % 256;
		count = read(fd, &buffer, 1);
		count = write (fd, chksum == buffer[0] ? commit : err,
                               strlen(commit));
	} while (chksum != buffer[0]);

	/* read filename */
	do {
		i = 0;
		chksum = 0;
		buffer[0] = ' ' ;
		while (i < 12 && buffer[0] >= ' ') {
			count = read(fd, &buffer[0], 1);
			if (buffer[0] >= ' ') {
				chksum = (chksum + buffer[0]) % 256;
				if (i < 12)
					filename[i++] = buffer[0];
			}
		}
		filename[i] = '\0';
		count = read(fd, &buffer, 1); 
		if (chksum != buffer[0])
			count = write (fd, err, strlen(err));
	} while (chksum != buffer[0]);
	if (strlen(filename) == 0)
		strcpy((char *)filename, "default.flx");

	/* open target file */
	fp = fopen(filename, "w");
	if (!fp) {
		fprintf(stderr, "error opening %s", filename);
		do_exit(1);
	}
	fprintf(stdout, "writing to %s\n", filename);
	if (fwrite(&flh, sizeof(struct s_flex_header), 1, fp) != 1) {
		fprintf(stderr, "error writing to %s\n", filename);
		fclose(fp);
		do_exit(1);
	}
	m = (long unsigned int)flh.magic_number;
	fprintf(stdout, "magic_number: %08lX\n", m);
	fprintf(stdout, "write_protect: %d\n", flh.write_protect);
	fprintf(stdout, "sizecode: %d\n", flh.sizecode);
	fprintf(stdout, "sides0: %d\n", flh.sides0);
	fprintf(stdout, "sectors0: %d\n", flh.sectors0);
	fprintf(stdout, "sides: %d\n", flh.sides);
	fprintf(stdout, "sectors: %d\n", flh.sectors);
	fprintf(stdout, "tracks: %d\n", flh.tracks);
	count = write(fd, commit, strlen(commit)); /* now commit */

	/* read track 0 */
	tr = 0;
	for (si=0; si < flh.sides0; si++) {
		for (se=0; se < flh.sectors0; se++) {
			for (blk=0; blk < BYTES_P_BLOCK; blk++) {
				errcnt = 0;
				do {
					i = 0;
					do {
						j = read(fd, &blk_buffer[i], BYTES_P_BLOCK + 1 - i);
						if (j > 0)
							i += j;
						else {
						   errcnt++;
						}
					} while (j > 0 && i <= 16);
					chksum = 0;
					for (i=0; i < BYTES_P_BLOCK; i++)
						chksum += blk_buffer[i];
					chksum %= 256;
					if (!chksum)
						chksum = 0x55;
					if (j <= 0 || chksum != blk_buffer[BYTES_P_BLOCK])
						count = write(fd, err, strlen(err));
					else
						count = write(fd, commit, strlen(commit));
					if (errcnt >= 10) {
						fprintf(stderr, "communication break down\n");
						do_exit(1);
					}
				} while(j <= 0 || chksum != blk_buffer[BYTES_P_BLOCK]);
				fwrite((void *)&blk_buffer, BYTES_P_BLOCK, 1, fp);
			} /* for blk */
		} /* for se */
	} /* for si */
		
	/* read track > 0 */
	for (tr=1; tr < flh.tracks; tr++) {
		for (si=0; si < flh.sides; si++) {
			for (se=0; se < flh.sectors; se++) {
				for (blk=0; blk < BYTES_P_BLOCK; blk++) {
					errcnt = 0;
					do {
						i = 0;
						do {
							j = read(fd, &blk_buffer[i], BYTES_P_BLOCK + 1 - i);
							if (j > 0)
								i += j;
							else {
							   errcnt++;
							}
						} while (j > 0 && i <= BYTES_P_BLOCK);
						chksum = 0;
						for (i=0; i < BYTES_P_BLOCK; i++)
							chksum += blk_buffer[i];
						chksum %= 256;
						if (!chksum)
							chksum = 0x55;
						if (j <= 0 || chksum != blk_buffer[BYTES_P_BLOCK])
							count = write(fd, err, strlen(err));
						else
							count = write(fd, commit, strlen(commit));
						if (errcnt >= 10) {
							fprintf(stderr, "communication break down\n");
							do_exit(1);
						}
					} while(j <= 0 || chksum != blk_buffer[BYTES_P_BLOCK]);
					fwrite((void *)&blk_buffer, BYTES_P_BLOCK, 1, fp);
				} /* for blk */
			} /* for se */
		} /* for si */
	} /* for tr */
	fprintf(stdout, "finished\n");		
        (void)count;
	do_exit(0);
	return 0;

} /* main */

#else
int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	fprintf(stderr, "rcvflex can only be compiled on Linux, sorry\n");
	return 1;
} /* main */

#endif

