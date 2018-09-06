#include <stdio.h>
#include <iostream>
using namespace std;

int BitCount (void *);
int
main (void)
{
  FILE *fpi2, *fpi, *fpt, *fpo;
  long of1;
  int i, delline = 0, nochange;
  unsigned char buf[60], tbuf[4];
  fpi2 = fopen ("wor16.fon", "rb");
  fpo = fopen ("wor16.fog", "wb");
  for (i = 0; i < 1666; i++)
    {
      fread (buf, 1, 1, fpi2);
      fwrite (buf, 1, 1, fpo);
    }
  fpt = fopen ("wor16s.asc", "rb");
  fpi = fopen ("hzk16", "rb");
  for (i = 0, nochange = 0; !feof (fpt); i += 2)
    {
      fseek (fpt, i, SEEK_SET);
      fread (tbuf, 1, 2, fpt);
      if (tbuf[0] <= 0xa0)
	{
	  fseek (fpi2, 1666L + i * 15L, SEEK_SET);
	  fread (buf, 1, 30, fpi2);
	  cout << 1666L + i * 15L;
	  delline = 0;
	  nochange++;
	}
      else
	{
	  of1 = (tbuf[0] - 161) * 94L + (tbuf[1] - 161);
	  //	  cout <<tbuf[0] <<tbuf[1] <<of1 * 32 <<char (delline + 'A');
	  fseek (fpi, of1 * 32, SEEK_SET);
	  fread (buf, 1, 32, fpi);
	  if (BitCount (buf) < BitCount (buf + 30))
	    delline = 2;
	  else
	    delline = 0;
	}
      fwrite (buf + delline, 1, 30, fpo);
    }
  fclose (fpi);
  fclose (fpo);
  fclose (fpt);
  fclose (fpi2);
  return 0;
}

int
BitCount (void *p)
{
  int s = 0, i;
  unsigned int w;
  w = *((unsigned int *) p);
  for (i = 0; i < 16; i++)
    if (w >>i & 1)
      s++;
  return s;
}
