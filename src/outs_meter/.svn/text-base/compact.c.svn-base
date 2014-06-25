#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#define setMax(old, this) if (old < this) old = this;
#define max(old, this) ((old) < (this) ? (this) : (old))

/**
 * i file da passare in ingresso hanno nome nel formato 
 *   Type_EsponteNscambi_NumHops_Impl.dat
 *   dove  Type in {0, 1, 2}
 *         EsponteNscambi = log10(nscambi)
 *
 * il formato delle righe dei file \`e il seguente
 *   Avg Sdev Max
 *
 * Viene prodotto un file con stesso nome di quello di ingresso
 * seguito da .avg contenente una tre righe con i tre valori medi
 * delle tre colonne del file di ingresso. Il formato del file:
 *   Avg Sdev Max
 */



int main(int argc, char **argv) {
  int j;
  /**
   * contengono media, sdev e max rispettivamente delle tre colonne 
   */
  double avg[3] = {0}, sdev[3] = {0}, max[3] = {0}, value;
  int count = 0, i;
  char str[512];
  FILE *in_file;
  FILE *out_file;
  printf("%d\n", argc);

  for (j=1; j<argc; j++) {
    for (i=0; i<3; i++) {
      avg[i] = 0; sdev[i] = 0; max[i] = 0;
    }
    count = i = 0;

    if (NULL == (in_file = fopen(argv[j], "r"))) {
      perror("fopen"); continue ;
    }
    sprintf(str, "%s.avg", argv[j]);
    if (NULL == (out_file = fopen(str, "w"))) {
      perror("fopen"); continue ;
    }

    while (EOF != fscanf(in_file, "%lf", &value)) {
      avg[i] += value;
      sdev[i] += value * value;
      setMax(max[i], value);
      if (i == 0) count ++;
      i = (i+1) % 3;
    }

    for (i=0; i<3; i++) {
      avg[i] /= count;
      sdev[i] = sqrt(sdev[i] / count - avg[i] * avg[i]);
      fprintf(out_file, "%f %f %f\n", avg[i], sdev[i], max[i]);
    }

    fclose(out_file);
    fclose(in_file);
  }

  return 0;
}
