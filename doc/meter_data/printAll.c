#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define CLOCK_CYCLE_NANOSEC 1.156626
#define CLOCK_CYCLE_MICROSEC 0.001156626
#define CLOCK_CYCLE_MILLSEC 0.000001156626

#define _openFile(filename, filepointer, mode, line) if (NULL == ((filepointer) = fopen((filename), mode))) { perror("fopen "_retval(line)); exit(EXIT_FAILURE); }
#define _retval(x) #x
#define openFile(filename, filepointer, mode) _openFile(filename, filepointer, mode, __LINE__)

/**
 * Il programma prende in ingresso da linea di comando due interi: il
 * tipo di misurazione (0, 1, 2) e il logaritmo base 10 del numero di
 * scambi effetu
 *
 * Vengono presi in ingresso i file
 * TYPE_NSCAMBLOG_*_ch_sym_ref_ad1_*.dat.avg
 * e viene prodotto in uscita il file TYPE_NSCAMBLOG_sym_all.dat.avg
 * contenente il valore medio di Lcom di tutti i canali simmetrici per
 * tutti i numeri di hops con cui si e` effettuata la misura.
 */

int main(int argc, char **argv) {
  int i;
  int type = atoi(argv[1]);
  int nscamb = atoi(argv[2]);
  char in_sym_str[512], in_asymin_str[512], out_sym_str[512], out_asymin_str[512], out2_sym_str[512], out2_asymin_str[512];
  int nhops[] = { 1, 8, 14};
  
  FILE *in_sym_file, *in_asymin_file, *out_sym_file, *out_asymin_file, *out2_sym_file, *out2_asymin_file;

  /* open sym result file and write the first row */
  sprintf(out_sym_str, "%d_%d_sym_all.dat.avg", type, nscamb);
  openFile(out_sym_str, out_sym_file, "w");
  fprintf(out_sym_file, "0 \\\\texttt{ch\\\\_sym\\\\_udn} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_rdyack\\\\_no} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_rdyack} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_nullack} \"uso della Rete di Interconnessione\" \"uso della Memoria Condivisa, coerenza cache predefinita\" \"uso della SM, configurazione coerenza cache\" \"uso della Memoria Condivisa, non uso di memory barrier\" \n");
  /* open asymin result file and write the first row */
  sprintf(out_asymin_str, "%d_%d_asymin_all.dat.avg", type, nscamb);
  openFile(out_asymin_str, out_asymin_file, "w");
  fprintf(out_asymin_file, "0 \\\\texttt{ch\\\\_asymin\\\\_udn} \\\\texttt{ch\\\\_asymin\\\\_sm\\\\_all} \\\\texttt{ch\\\\_asymin\\\\_sm} \"uso della Rete di Interconnessione\" \"uso della memoria condivisa, allocati tutti i mittenti\" \"uso della memoria condivisa, allocato un unico mittente\" \n");

  /* open sym result file with statistics and write the first row */
  sprintf(out2_sym_str, "%d_%d_sym_all_stat.dat.avg", type, nscamb);
  openFile(out2_sym_str, out2_sym_file, "w");
  fprintf(out2_sym_file, "0 \\\\texttt{ch\\\\_sym\\\\_udn} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_rdyack\\\\_no} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_rdyack} \\\\texttt{ch\\\\_sym\\\\_sm\\\\_nullack}\n");
  sprintf(out2_asymin_str, "%d_%d_asymin_all_stat.dat.avg", type, nscamb);
  /* open asymin result file with statistics and write the first row */
  openFile(out2_asymin_str, out2_asymin_file, "w");
  fprintf(out2_asymin_file, "0 \\\\texttt{ch\\\\_asymin\\\\_udn} \\\\texttt{ch\\\\_asymin\\\\_sm\\\\_all} \\\\texttt{ch\\\\_asymin\\\\_sm}\n");

  /* Per ogni tipo di canale e ogni possibile valore di numero di hops
   * (valori nell'array nhops) viene aperto il corrispondente file
   * contenente le statistiche delle misurazioni di Lcom e letti i
   * valori di media devianza e massimo valore.
   * Il valore di valor medio viene scritto in T_S_sym_all.dat.avg
   * (out_sym_file), tutti e tre i valori vengono scritti nel file
   * T_S_sym_all_stat.dat.avg (out2_sym_file) 
   */
  for (i=0; i<3; i++) {
    double value[3];

    // replace "%f ", value[0]    con    "%f %f ", value

    /* symmetric channels */
    fprintf(out_sym_file, "%d ", nhops[i]);
    fprintf(out2_sym_file, "%d ", nhops[i]);

    sprintf(in_sym_str, "%d_%d_%d_ch_sym_ref_ad1_udn.dat.avg", type, nscamb, nhops[i]);
    openFile(in_sym_str, in_sym_file, "r");
    fscanf(in_sym_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_sym_file, "%f ", value[0]);
    fprintf(out2_sym_file, "%f %f %f ", value[0], value[1], value[2]);    
    fclose(in_sym_file);
    sprintf(in_sym_str, "%d_%d_%d_ch_sym_ref_ad1_sm_no.dat.avg", type, nscamb, nhops[i]);
    openFile(in_sym_str, in_sym_file, "r");
    fscanf(in_sym_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_sym_file, "%f ", value[0]);
    fprintf(out2_sym_file, "%f %f %f ", value[0], value[1], value[2]);    
    fclose(in_sym_file);
    sprintf(in_sym_str, "%d_%d_%d_ch_sym_ref_ad1_sm_fence.dat.avg", type, nscamb, nhops[i]);
    openFile(in_sym_str, in_sym_file, "r");
    fscanf(in_sym_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_sym_file, "%f ", value[0]);
    fprintf(out2_sym_file, "%f %f %f ", value[0], value[1], value[2]);    
    fclose(in_sym_file);
    sprintf(in_sym_str, "%d_%d_%d_ch_sym_ref_ad1_sm_nullack.dat.avg", type, nscamb, nhops[i]);
    openFile(in_sym_str, in_sym_file, "r");
    fscanf(in_sym_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_sym_file, "%f ", value[0]);
    fprintf(out2_sym_file, "%f %f %f ", value[0], value[1], value[2]);    
    fclose(in_sym_file);

    fprintf(out_sym_file, "\n");
    fprintf(out2_sym_file, "\n");

    /* asymmetric channels */
    fprintf(out_asymin_file, "%d ", nhops[i]);
    fprintf(out2_asymin_file, "%d ", nhops[i]);

    sprintf(in_asymin_str, "%d_%d_%d_ch_asymin_ref_ad1_udn.dat.avg", type, nscamb, nhops[i]);
    openFile(in_asymin_str, in_asymin_file, "r");
    fscanf(in_asymin_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_asymin_file, "%f ", value[0]);
    fprintf(out2_asymin_file, "%f %f %f ", value[0], value[1], value[2]);
    fclose(in_asymin_file);
    sprintf(in_asymin_str, "%d_%d_%d_ch_asymin_ref_ad1_sm_param.dat.avg", type, nscamb, nhops[i]);
    openFile(in_asymin_str, in_asymin_file, "r");    
    fscanf(in_asymin_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_asymin_file, "%f ", value[0]);
    fprintf(out2_asymin_file, "%f %f %f ", value[0], value[1], value[2]);
    fclose(in_asymin_file);
    sprintf(in_asymin_str, "%d_%d_%d_ch_asymin_ref_ad1_sm_param_b.dat.avg", type, nscamb, nhops[i]);
    openFile(in_asymin_str, in_asymin_file, "r");
    fscanf(in_asymin_file, "%lf %lf %lf", value, value+1, value+2);
    fprintf(out_asymin_file, "%f ", value[0]);
    fprintf(out2_asymin_file, "%f %f %f ", value[0], value[1], value[2]);
    fclose(in_asymin_file);

    fprintf(out_asymin_file, "\n");
    fprintf(out2_asymin_file, "\n");
  }

  fclose(out_sym_file);
  fclose(out_asymin_file);
  return 0;
}
