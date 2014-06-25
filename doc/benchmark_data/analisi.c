#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "error_handler.h"

int main(int argc, char ** argv) {
  char buffer[1024];

  char filename[128];
  int Ta, impl, size, nopt = 0, flag = 0;
  double Tcalc;
  double deltacom_udn = 180.95523;
  double deltacom_sm = 724.91273000000001;

  int i;
  FILE *input;
  struct t {int n; double value; double s;} max, near_up_nopt, near_down_nopt;
  double a[9];

  for (i=1; i<argc; i++) {
    flag = 0;
    bzero(&max, sizeof(max));
    ERRHAND_NN(input = fopen(argv[i], "r"));

    strcpy(filename, argv[i]);
    strtok(filename, "_"); //version
    impl = atoi(strtok(NULL, "_")); //implementation
    strtok(NULL, "_"); //stream size
    Ta = atoi(strtok(NULL, "_")); //interarrival
    size = atoi(strtok(NULL, "_")); //size
    if (size == 56) Tcalc = 82934.926829000004;
    else if (size == 168) Tcalc = 735319.29268299998;
    else if (size == 280) Tcalc = 2040430.560976;
    if (impl == 0) 
      nopt = ceil(Tcalc / (Ta - deltacom_udn));
    else if (impl == 11)
      nopt = ceil(Tcalc / (Ta - deltacom_sm));

    //printf("DBG: %d %d %d %f %d\n", impl, Ta, size, Tcalc, nopt);

    fgets(buffer, 1024, input);
    fgets(buffer, 1024, input);
    fgets(buffer, 1024, input);
    while (EOF != fscanf(input, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", a, a+1, a+2, a+3, a+4, a+5, a+6, a+7, a+8)) {
      //printf(">>>>>> %f %d \n", a[5], nopt);
      if (flag == 0 && a[0] <= nopt) {
	near_down_nopt.n = a[0]; near_down_nopt.value = a[1]; near_down_nopt.s = a[5]; 
      } else if (flag != 2 && a[0] > nopt) { 
	flag = 2; near_up_nopt.n = a[0]; near_up_nopt.value = a[1]; near_up_nopt.s = a[5]; 
      }
      //if (a[5] > max.s) {
      if (a[0] == 56) {
	max.n = a[0]; max.value = a[1]; max.s = a[5]; 
      }
    }
    printf("%s %d %f %f NOPT=%d\n", argv[i], max.n, max.value, max.s, nopt);
    printf("%d & %.3f & %.3f \\\\\n", max.n, max.s, max.value);
    printf("%d & %.3f & %.3f \\\\\n", near_down_nopt.n, near_down_nopt.s, near_down_nopt.value);
    printf("%d & %.3f & %.3f \\\\\n", near_up_nopt.n, near_up_nopt.s, near_up_nopt.value);
    printf("& & & & \\clr %d & \\clr %.3f & \\clr %.3f \\\\\n", near_up_nopt.n, near_up_nopt.s, near_up_nopt.value);
    fclose(input);
  }
       
  return 0;
}
