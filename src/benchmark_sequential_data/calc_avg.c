#include <stdlib.h>
#include <stdio.h>
#include "error_handler.h"

int main() {
  FILE *input_file, *output_file;
  double a[3], c[3] = { 0 }, count = 0;
  ERRHAND_NN(output_file = fopen("calculateTime.dat.avg", "w"));
  ERRHAND_NN(input_file = fopen("calculateTime.dat", "r"));
  while (EOF != fscanf(input_file, "%lf %lf %lf", a, a+1, a+2)) {
    c[0] += a[0]; c[1] += a[1]; c[2] += a[2];
    count ++;
  }
  fprintf(output_file, "%f %f %f\n", c[0]/count, c[1]/count, c[2]/count);
  fclose(output_file);
  return 0;
}
