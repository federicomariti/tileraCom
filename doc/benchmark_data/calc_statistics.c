#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "error_handler.h"

//titolo del file risultato: "Matrix Size 100x100"
#define TITLE "Matrix size "
#define COMMENTS "#paralDegree avg max min sdev scalability 1/scalability"
#define MAX_CPU 64
#define CLOCK_CYCLE_NANOSEC 1.156626
#define CLOCK_CYCLE_MICROSEC 0.001156626
#define CLOCK_CYCLE_MILLSEC 0.000001156626
//0,864583711588707
typedef struct {
  double sum;
  double ssum;
  double max;
  double min;
  unsigned int count;
} record_t;

//acceduto per parallelDegree
record_t array[MAX_CPU+1]; //24*64

typedef struct {
  double avg;
  double scal;
  double avg_multicast;
} save_t;

int main(int argc, char **argv)
{
  int i, j;
  char resultfile_name[128], resultfile_name_2[128], sep[] = "_.";
  char buffer[128];
  double avg_seq;
  FILE *datafile;
  FILE *resultfile;

  save_t statistics[MAX_CPU+1];
  int max_n = 0;

  double millis, micros;

  double Tcalc_all[3] = {82934.926829, 735319.292683, 2040430.560976};
  double Tcalc_microsec_all[3] = {95.924692678518994, 850.48941221876782, 2360.0150380194277};
  double Tcalc;

  /* itera tutti i file di dati */
  for (i=1; i<argc; i++) {
    int n, datasize, streamSize, interarrival, impl;
    char *vers, *impl_str, *impl_str_2;
    double tc;

    memset(array, 0, sizeof(array));
    memset(statistics, 0, sizeof(statistics));

    strcpy(resultfile_name, argv[i]);
    strcat(resultfile_name, ".avg");

    strcpy(resultfile_name_2, argv[i]);
    strcat(resultfile_name_2, ".nb-multictime.avg");

    ERRHAND_NN(datafile = fopen(argv[i], "r"));
    ERRHAND_NN(resultfile = fopen(resultfile_name, "w"));

    /* leggi tutti i record e crea le statistiche per i vari gradi di
     * parallelismo */
    while (NULL != fgets(buffer, 127, datafile)) {
      sscanf(buffer, "%d %lf\n", &n, &tc);
      if ('#' == buffer[0]) continue;
      if ('\0' == buffer[0]) continue;
      if (max_n < n) max_n = n;
      array[n].count ++;
      array[n].sum += tc;
      array[n].ssum += tc*tc;
      if (array[n].max < tc) array[n].max = tc;
      if (array[n].min > tc || array[n].min == 0) array[n].min = tc;
    }

    printf("%s: \n", argv[i]);
    for (j=0; j<MAX_CPU; j++)
      if (array[j].count) { printf("n=%d count=%d  ", j, array[j].count); break; }
    printf("\n");

    fclose(datafile);

    /* nogatherInt_00_100_1_40.dat */
    vers = strtok(argv[i], sep);    /* version */
    vers+= 8;
    impl = atoi(strtok(NULL, sep)); /* implementation */
    switch (impl) {
    case 0: impl_str = "udn udn"; impl_str_2 = "uso della Rete di Interconnessione"; break;
    case 11: impl_str = "sm sm"; impl_str_2 = "uso della Memoria Condivisa"; break;
    case 01: impl_str = "udn, sm"; break;
    }
    streamSize = atoi(strtok(NULL, sep));	  /* stream size */
    interarrival = atoi(strtok(NULL, sep));	  /* interarrival */
    datasize = atoi(strtok(NULL, sep));		  /* data size */

    switch(datasize) {
    case 56: Tcalc = Tcalc_all[0];  avg_seq = Tcalc_all[0]; break;
    case 168: Tcalc = Tcalc_all[1]; avg_seq = Tcalc_all[1]; break;
    case 280: Tcalc = Tcalc_all[2]; avg_seq = Tcalc_all[2]; break;
    }

    /* scrivi il titolo nel resultfile.
     *
     * possibili titoli
     * - grafico scalabilita con diversi valori di M => matrix size
     * - grafico confronti tempo calcolo riga tra implementaz diverse dei canali => implementazione canali
     * - grafico confronti scalabilita` tra tipi int e float => tipo dati
     * - grafico confronti con diverso tempo di interarrivo => interarrivo
     * - grafico confronti con diverse lunghezze dello stream => lungh. stream
     */
    //fprintf(resultfile, "\"Matrix Size %dx%d\" \"%s\" \"%s\" \"stream length %d\"\n", datasize, datasize, impl_str, vers, streamSize);
    fprintf(resultfile, "\"%s\" \"%s\" \"stream length %d\" \"$T_A$ %d $\\\\tau$\" \"Matrix Size %dx%d\" \"$T_A$ %.3f $\\\\mu\\\\mathrm{sec}$\" \"%s\" \n",
	    vers, impl_str, streamSize, interarrival, datasize, datasize,
	    interarrival*CLOCK_CYCLE_MICROSEC,
	    impl_str_2
	    );
    /* scrivi i commenti nel resultfile */
    fprintf(resultfile,
	    "#Channels impl = %s, Stream Size = %d, Interarrival Serivce Time = %d, Data Size = %d\n"COMMENTS"\n",
	    impl_str, streamSize, interarrival, datasize);
    /* scrivi le statistiche dei risultati nel file resultfile */
    for (j=1; j<MAX_CPU; j++)
      if (0 != array[j].count) {
	double avg = array[j].sum / array[j].count;
	double sdev = sqrt(array[j].ssum / array[j].count - avg * avg);
	double scal, scalInv;

	if (1 == j) {
	  scal = 1;
	  scalInv = 1;
	  avg_seq = avg;
	} else {
	  scal = avg_seq / avg;
	  scalInv = avg / avg_seq;
	}

	statistics[j].avg = avg;
	statistics[j].scal = scal;

	millis = avg * CLOCK_CYCLE_MILLSEC;
	micros = avg * CLOCK_CYCLE_MICROSEC;

	fprintf(resultfile, "%d %f %f %f %f %f %f %f %f\n",
		j, avg, array[j].max, array[j].min, sdev, scal, scalInv, 
		millis, micros);
      }

    fclose(resultfile);

    /* verifica che il file contenga i dati del tempo di completamento
     * (in milliscondi) */
    int l = strlen(resultfile_name);
    if (!strcmp(resultfile_name+l-8, ".dat.avg") && 'e' != resultfile_name[l-9]) {
      /* Se esiste, apri in lettura il file delle statistiche dei
       * tempi di servizio della multicast corrispondente alla stessa
       * computazione.
       *
       * Verranno letti i valori del tempo di multicast con grado di
       * parallelismo per il quale la Map non e` collo di
       * bottiglia. Tali tempi sono scritti in un file '*.no-avg'. */
      double idealCompleteTime = interarrival * streamSize * CLOCK_CYCLE_MILLSEC;
      resultfile_name[l-8] = '\0';
      strcat(resultfile_name, "_multictime.dbg.avg");

      if (NULL != (datafile = fopen(resultfile_name, "r"))) {
	int flag_exists_n_st_nb = 0;
	char firstRow[128];
	fgets(firstRow, 127, datafile);
	fgets(buffer, 127, datafile);
	fgets(buffer, 127, datafile);
	while (NULL != fgets(buffer, 127, datafile)) {
	  int n;
	  double tmulticast;
	  sscanf(buffer, "%d %lf", &n, &tmulticast);
	  /* printf("[DBG][%s] n = %d Tc = %f Tc_id = %f Ta = %d K = %d \n", */
	  /* 	 resultfile_name, n, statistics[n].avg, idealCompleteTime, */
	  /* 	 interarrival, streamSize); */
	  if ((n < max_n || 1 == flag_exists_n_st_nb)
	      && statistics[n].avg > idealCompleteTime - 0.6 
	      && statistics[n].avg < idealCompleteTime + 0.6) {
	    statistics[n].avg_multicast = tmulticast;
	    flag_exists_n_st_nb = 1;
	  }
	}

	if (flag_exists_n_st_nb) {
	  /* scrivi i valori del tempo di multicast con Map non collo
	   * di bottiglia in un file separato */
	  ERRHAND_NN(resultfile = fopen(resultfile_name_2, "w"));
	  fprintf(resultfile, "%s", firstRow);
	  for (j=1; j<MAX_CPU; j++)
	    if (0 != statistics[j].avg_multicast) {
	      millis = statistics[j].avg_multicast * CLOCK_CYCLE_MILLSEC;
	      micros = statistics[j].avg_multicast * CLOCK_CYCLE_MICROSEC;
	      fprintf(resultfile, "%d %f %f %f\n", 
		      j, statistics[j].avg_multicast, millis, micros);
	    }
	  fclose(resultfile);
	}
      }
    }    
  } /* end for (i=1; i<argc; i++) */

  return 0;
}
