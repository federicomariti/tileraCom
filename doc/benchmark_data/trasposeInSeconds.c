#include <stdlib.c>
#include <stdio.h>
#include <errno.h>

#include "error_handler.h"

/* frequency = 864.5837116 Mhz */
#define CLOCK_CYCLE_NANOSEC 1.156626
#define CLOCK_CYCLE_MILLSEC 0.000001156626

int main(int argc, char **argv) 
{
  int i;
  char str[512];
  FILE *input_file, *output_file;

  for (i=1; i<argc; i++) {
    strcpy(str, argv[i]);
    strcat(argv[i], ".transposed");
    ERRHAND_NN(input_file = fopen(argv[i], "r"));
    ERRHAND_NN(output_file = fopen(, "w"));
    fscanf(input_file, "%d %f", &n, &value);
    fprintf(output_file, "%d %f", n, value);
    fclose(output_file);
    flcose(input_file);
  }
 
 return 0;
}
	       
