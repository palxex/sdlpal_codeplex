#include <stdlib.h>
#include <stdio.h>

int
main()
{
  FILE *fpASC, *fpFON, *fpSTD, *fpSPC;
  unsigned char c[2], f[30];
  unsigned short s;
  int i;

  fpASC = fopen("wor16.asc", "rb");
  fpFON = fopen("wor16.fon", "wb");
  fpSTD = fopen("stdfont.15", "rb");
  fpSPC = fopen("spcfont.15", "rb");

  for (i = 0; i < 1666; i++)
    fputc(0, fpFON);

  while (!feof(fpASC))
    {
      fread(c, sizeof(c), 1, fpASC);
      s = (c[0] << 8) + c[1];

      if (c[1] > 0xa0)
	s -= 34;

      if (s >= 0xc940)
	{
	  s -= (c[0] - 0xc9) * 99;
	  fseek(fpSTD, (long)(s - 0xc940 + 5401) * 30, SEEK_SET);
	  fread(f, sizeof(f), 1, fpSTD);
	}
      else if (s >= 0xa440)
	{
	  s -= (c[0] - 0xa4) * 99;
	  fseek(fpSTD, (long)(s - 0xa440) * 30, SEEK_SET);
	  fread(f, sizeof(f), 1, fpSTD);
	}
      else
	{
	  s -= (c[0] - 0xa1) * 99;
	  fseek(fpSPC, (long)(s - 0xa140) * 30, SEEK_SET);
	  fread(f, sizeof(f), 1, fpSPC);
	}

      fwrite(f, sizeof(f), 1, fpFON);
    }

  fclose(fpASC);
  fclose(fpFON);
  fclose(fpSTD);
  fclose(fpSPC);

  return 0;
}
