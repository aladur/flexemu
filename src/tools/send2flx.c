/*

 send2flx.c

program to transfer one file through serial port from
UNIX to FLEX

Copyright (C) W. Schwotzer 2003
*/


#include <stdio.h>

#if defined (__LINUX) || defined (__BSD)

#include <ctype.h>
#include <sys/ioctl.h>
#ifdef __LINUX
  #include <termio.h>
#endif
#ifdef __BSD
  #include <termios.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_DEVICE	"/dev/ttyS0"
#define MAX_FILESIZE    (0xC000)

struct f_header
{
	unsigned char size[2];
	char filename[8];
	char extension[3];
	char flags;
};

typedef enum {
	FT_GUESS,
	FT_BINARY,
	FT_TEXT
} eFileType;

FILE 			*fp;	/* file pointer to target file*/
int			fd;	/* file descriptor for serial device*/
struct termios 		oldtty;


int init_serial_port(const char *device, int speed, struct termios *pOldtty)
{
	int    new_fd;
	struct termios ntty;

	new_fd = open(device, O_RDWR | O_NOCTTY);
	if (new_fd == -1)
	{
		fprintf(stderr, "Unable to open device %s\n", device);
		return -1;
	}
        if (!isatty(new_fd))
	{
		fprintf(stderr, "%s is no tty device\n", device);
		close(new_fd);
		return -1;
	}

	if (tcgetattr(new_fd, pOldtty) == -1) {
		fprintf(stderr, "%s is wrong device\n", device);
		close(new_fd);
		return -1;
	}
	/* memcpy(&ntty, &oldtty, sizeof(struct termios)); */
	memset(&ntty, 0, sizeof(struct termios));
	ntty.c_iflag = 0;
	ntty.c_oflag = 0;
	ntty.c_cflag = (tcflag_t)(CLOCAL | CREAD | CS8);
	ntty.c_lflag = NOFLSH;
	ntty.c_cc[VMIN] = 0;
	ntty.c_cc[VTIME] = 50;
	cfsetispeed(&ntty, speed);
	cfsetospeed(&ntty, speed);
	
	if (tcsetattr(new_fd, TCSANOW, &ntty) == -1) {
		fprintf(stderr, "can't set serial line %s\n", device);
		tcsetattr(new_fd, TCSANOW, pOldtty);
		close(new_fd);
		return -1;
	}
	return new_fd;
}


void do_exit(struct termios *pOldtty, int exit_code)
{
	if (fp != NULL)
		fclose(fp);
	tcsetattr(fd, TCSANOW, pOldtty);
	close(fd);
	exit(exit_code);
} /* do_exit */

#ifdef sun
	void sigint(int param, ...)
#else
	void sigint(int param)
#endif
{
        (void)param; /* satisfy compiler */
	do_exit(&oldtty, 2);
} /* sigint */

void init_header(struct f_header *pHeader,
	const char *filename,
	int size,
	eFileType filetype,
	short overwrite)
{
	int i, j = 0;

	memset(pHeader, 0, sizeof(struct f_header));
	if (strrchr(filename, '/') != NULL)
		j = strrchr(filename, '/') - filename + 1;
	for (i = 0; i < 8 && filename[i+j] != '\0' && filename[i+j] != '.'; i++)
		pHeader->filename[i] = toupper(filename[i+j]);
	if (strrchr(filename, '.') != NULL &&
            strrchr(filename, '.') > strrchr(filename, '/'))
	{
		j = strrchr(filename, '.') - filename + 1;
		for (i = 0; i < 3 && filename[i+j] != '\0'; i++)
			pHeader->extension[i] = toupper(filename[i+j]);
	}
	pHeader->size[0] = (unsigned char)(size >> 8); 
	pHeader->size[1] = (unsigned char)(size & 0xFF); 
	if (overwrite)           pHeader->flags |= 0x01;
	if (filetype == FT_TEXT) pHeader->flags |= 0x02;
}

FILE *init_file(const char *filename, int *size)
{
	struct stat st;
	FILE *new_fp = NULL;

        if (stat(filename, &st) == -1)
	{
		fprintf(stderr, "Error getting file info for %s\n", filename);
		return new_fp;
	}
	new_fp = fopen(filename, "r");
	if (new_fp == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", filename);
		return new_fp;
	}
        if (st.st_size > MAX_FILESIZE)
	{
		fprintf(stderr, "File is too long %s\n", filename);
		fclose(new_fp);
		return NULL;
	}
	*size = st.st_size;
	return new_fp;
}

void read_file(unsigned char *buffer, eFileType *filetype, int *filesize)
{
	if (*filetype == FT_GUESS)
	{
		int i, controlchars = 0;
		size_t count = fread(buffer, 1, *filesize, fp);
                (void)count; /* satisfy compiler */
		for (i = 0; i < *filesize; i++)
			if (buffer[i] < ' ' || buffer[i] > 0x7F)
				controlchars++;
		*filetype = controlchars * 100 / *filesize > 10 ?
			FT_BINARY : FT_TEXT;
		rewind(fp);
	}
	if (*filetype == FT_BINARY)
	{
		*filesize = fread(buffer, 1, *filesize, fp);
		return;
	}
	if (*filetype == FT_TEXT)
	{
		short sc = 0;
		int spaces = 0;
		int i = 0;
		int c;
	        while ((c = fgetc(fp)) != EOF && i < MAX_FILESIZE - 3) {
        	        if (c != ' ' && c != '\t' && spaces) {
                	        if (sc && spaces > 1) {
                        	        buffer[i++] = 0x09;
                        	        buffer[i++] = spaces;
				} else
                        	        buffer[i++] = ' ';
                        	spaces = 0;
                	}
         	       if (c == ' ') {
                	        /* do space compression */
                        	if (sc && ++spaces == 127) {
                        	        buffer[i++] = 0x09;
                        	        buffer[i++] = spaces;
        	                        spaces = 0;
                        	} else
					buffer[i++] = c;
                                
	                } else
        	        if (c == '\t') {
                	        /* tab will be converted to 8 spaces */
                        	if (sc)
				{
					if (spaces >= 127 - 8) {
                        	        	buffer[i++] = 0x09;
                        	        	buffer[i++] = 127;
        	                        	spaces -= 127 - 8;
                	        	} else
                        	        	spaces += 8;
				} else {
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
					buffer[i++] = ' ';	
				}	
	                } else
        	        if (c == '\n')
                      	        buffer[i++] = 0x0d;
               	 	else
                      	        buffer[i++] = c;
        	} /* while */
        	if (spaces) {
                        buffer[i++] = 0x09;
                        buffer[i++] = spaces;
        	}
		*filesize = i;
	}
}

void write_serial(int fd_out, const unsigned char *buffer, int size)
{
	int i;
	unsigned char checksum;

	checksum = 0;
	for (i = 0; i < size; i++)
		checksum += buffer[i];
	ssize_t count = write(fd_out, buffer, size);
	count = write(fd_out, &checksum, 1);
        (void)count; /* satisfy compiler */
}

void print_help(const char *device)
{
	printf("syntax: send2flx [-o][-t|-b] [-d <device>] -f <filename>\n");
	printf(" Send a file over a serial device to FLEX\n");
	printf(" Within FLEX RCVFILE.CMD can be used to save the file to disk\n");
	printf(" -o Overwrite file on FLEX without user confirmation\n");
	printf(" -q Quiet mode\n");
	printf(" -t File is a Text file. Space compression is done\n");
	printf(" -b File is a binary file. File is not converted\n");
	printf(" -d <device> Use <device> for serial communication\n");
        printf("             %s is the default device\n", device);
	printf(" -f <filename> The file to be transmitted to FLEX\n");
	printf("    If neither -t nor -b is given send2flx tries to guess\n");
	printf("    the file format\n");
}

int main(int argc, char *argv[])
{
	char	*device; /* pointer to serial device	*/
	unsigned char buffer[MAX_FILESIZE];
	int result;
	int filesize;
	char *filename;
	eFileType filetype;
	short overwrite;
	short quiet;
	short help;
	struct f_header header;
	unsigned char sync[4] = { 0x55, 0x55, 0x55, 0xAA };

	filename = "";
	overwrite = 0;
	quiet     = 0;
	help      = 0;
	filetype  = FT_GUESS;
	device = DEFAULT_DEVICE;
        fd = -1;
	fp = NULL;

	while (1)
	{
		result = getopt(argc, argv, "qotbhvd:f:");
		if (result == -1) break;
		switch (result)
		{
		case 'h': help      = 1;           break;
		case 'o': overwrite = 1;           break;
		case 'b': filetype  = FT_BINARY;   break;
		case 't': filetype  = FT_TEXT;     break;
		case 'd': device    = optarg;      break;
		case 'q': quiet     = 1;           break;
		case 'f': filename  = optarg;      break;
		case 'v': printf("V1.0"); exit(0); break;
		}
	}	

	if (help || argc <= 1 || strlen(filename) == 0)
	{
		print_help(DEFAULT_DEVICE);
		exit(0);
	}

	signal (SIGINT, sigint);
	
	fd = init_serial_port(device, B19200, &oldtty);
	if (fd == -1)
		exit(1);

	fp = init_file(filename, &filesize);
	if (fp == NULL)
		do_exit(&oldtty, 1);
	read_file(buffer, &filetype, &filesize);
	if (!quiet)
	{
		printf("filetype: %s\n",
			filetype == FT_TEXT ? "Textfile" : "Binary file");
		printf("filesize: %d\n", filesize);
	}
	/* tcflush(fd, TCIOFLUSH); flush input/output */
        ssize_t count = write(fd, sync, 4);
        (void)count; /* satisfy compiler */
	init_header(&header, filename, filesize, filetype, overwrite);
	write_serial(fd, (unsigned char *)&header, sizeof(struct f_header)); 
	sleep(1);
	write_serial(fd, buffer, filesize);
	tcdrain(fd); /* wait until serial data written (important!) */
	do_exit(&oldtty, 0);
	return 0;

} /* main */

#else
#warning "send2flx can only be compiled on Linux, sorry"
int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	return 1;
}
#endif /* ifdef linux */

