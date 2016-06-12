/* conv.c

	simple program for doing some fileformatconversions.
*/


#include <stdio.h>

FILE *fp1, *fp2;
char buffer[16];
int ch;

void error(void)
{
	fprintf(stderr, "error\n");
	exit(1);
}


main(int argc, char *argv[])
{
	if (argc != 3)
		error();
	fp1 = fopen(argv[1], "r");
	if (!fp1) 
		error();
	fp2 = fopen(argv[2], "w");
	if (!fp2) 
		error();
	fputc(0x33, fp2);
	fputc(0x9a, fp2);
	fputc(0x5c, fp2);
	fputc(0x48, fp2);
	fputc(0x00, fp2);
	fread((char *)&buffer, 6, 1, fp1);
	fwrite((char *)buffer, 6, 1, fp2);
	fputc('\0', fp2);
	fputc('\0', fp2);
	fputc('\0', fp2);
	fputc('\0', fp2);
	fputc('\0', fp2);
	ch = fgetc(fp1);
	while (ch != EOF) {
		fputc(ch, fp2);
		ch = fgetc(fp1);
	};
	fclose(fp1);
	fclose(fp2);
}  /* main */

