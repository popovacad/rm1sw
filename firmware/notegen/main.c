#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(int argc, char **argv)
{
  char nname[4];
  int timrel;

  printf("// Notes\n\n");

  for (int i = 0; i <= 36; i++)
  {
    timrel = round(16000000.0/(440.0*pow(2.0,((i-12.0)/12.0)))/2.0);

    switch (i%12)
    {
      case 0:
        memcpy(nname, "A\0\0\0", 4);
        break;
      case 1:
        memcpy(nname, "Ais\0", 4);
        break;
      case 2:
        memcpy(nname, "B\0\0\0", 4);
        break;
      case 3:
        memcpy(nname, "C\0\0\0", 4);
        break;
      case 4:
        memcpy(nname, "Cis\0", 4);
        break;
      case 5:
        memcpy(nname, "D\0\0\0", 4);
        break;
      case 6:
        memcpy(nname, "Dis\0", 4);
        break;
      case 7:
        memcpy(nname, "E\0\0\0", 4);
        break;
      case 8:
        memcpy(nname, "F\0\0\0", 4);
        break;
      case 9:
        memcpy(nname, "Fis\0", 4);
        break;
      case 10:
        memcpy(nname, "G\0\0\0", 4);
        break;
      case 11:
        memcpy(nname, "Gis\0", 4);
        break;
    }

    printf("#define %s%i %i\n", nname, (int)trunc(i/12.0), timrel);
  }

}
