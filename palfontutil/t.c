#include <stdio.h>

short sss[100000];
int num = 0;

main ()
{
  FILE *fp;

  fp = fopen ("wor16.asc", "rb");

  while (!feof (fp))
    {
      fread (&sss[num], sizeof (short), 1, fp);
      num++;
    }

  fclose (fp);

  fp = fopen ("desc.dat", "rb");

  while (!feof (fp))
    {
      unsigned char c = fgetc (fp), c2;
      short ssss;
      int i;
      if (c < 0x80)
	continue;
      c2 = fgetc (fp);
      ssss = (c2 << 8) | c;
      for (i = 0; i < num; i++)
	if (sss[i] == ssss)
	  break;
      if (i < num)
	continue;
      sss[num++] = ssss;
    }

  fclose (fp);
  fp = fopen ("wor161.asc", "wb");

  fwrite (sss, sizeof (short), num, fp);
  fclose (fp);
}
