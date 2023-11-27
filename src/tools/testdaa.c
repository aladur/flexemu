#include <stdio.h>
#define Byte unsigned char
#define Word unsigned short

int  daa(Byte a, Byte *status)
{
        Byte    hc = (*status & 0x20);
        Byte    carry = (*status & 0x01);
        Word    t, c = 0;
        Byte    lsn = a & 0x0f;
        Byte    msn = a & 0xf0;
        Byte    r;

        if (hc || (lsn > 9)) {
                c |= 0x06;
        }
        if (  carry    ||
             (msn > 0x90) ||
            ((msn > 0x80) && (lsn > 9))) {
                c |= 0x60;
        }

        t = c + a;
        *status = 0;
        if (carry || t & 0x100) *status |= 0x01;
        if (t & 0x80) *status |= 0x08;
        if ((t & 0xFF) == 0) *status |= 0x04;
        r = (Byte)t;
        return r;
}

int main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;

   Byte r;
   int count = 0;
   Byte a = 0;
   Byte cc = 0;
   fprintf(stdout, "Carry: 0, Halfcarry: o\n");
   for (count = 0; count < 256; count++)
   {
      cc = 0;
      r = daa(a, &cc);
      fprintf(stdout, "%02X%02X ", r, cc);
      a += 1;
   }
   fprintf(stdout, "Carry: 1, Halfcarry: o\n");
   for (count = 0; count < 256; count++)
   {
   cc = 1;
      r = daa(a, &cc);
      fprintf(stdout, "%02X%02X ", r, cc);
      a += 1;
   }
   fprintf(stdout, "Carry: 0, Halfcarry: 1\n");
   for (count = 0; count < 256; count++)
   {
   cc = 0x20;
      r = daa(a, &cc);
      fprintf(stdout, "%02X%02X ", r, cc);
      a += 1;
   }
   fprintf(stdout, "Carry: 1, Halfcarry: 1\n");
   for (count = 0; count < 256; count++)
   {
   cc = 0x21;
      r = daa(a, &cc);
      fprintf(stdout, "%02X%02X ", r, cc);
      a += 1;
   }
   return 0;
}

