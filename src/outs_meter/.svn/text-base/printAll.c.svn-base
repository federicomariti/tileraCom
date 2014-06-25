#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define _openFile(filename, filepointer, mode, line) if (NULL == ((filepointer) = fopen((filename), mode))) { perror("fopen "_retval(line)); exit(EXIT_FAILURE); }
#define _retval(x) #x
#define openFile(filename, filepointer, mode) _openFile(filename, filepointer, mode, __LINE__)



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
  //fprintf(out_sym_file, "hops ch\\_sym\\_udn ch\\_sym\\_sm\\_no ch\\_sym\\_sm ch\\_sym\\_sm\\_nullack\n");
  fprintf(out_sym_file, "0 a b c d e f\n");
  /* open asymin result file and write the first row */
  sprintf(out_asymin_str, "%d_%d_asymin_all.dat.avg", type, nscamb);
  openFile(out_asymin_str, out_asymin_file, "w");
  //fprintf(out_asymin_file, "hops ch\\_asymin\\_udn ch\\_asymin\\_sm ch\\_asymin\\_sm\\_all\n");
  fprintf(out_asymin_file, "0 a b c\n");

  sprintf(out2_sym_str, "%d_%d_sym_all_stat.dat.avg", type, nscamb);
  openFile(out2_sym_str, out2_sym_file, "w");
  //fprintf(out2_sym_file, "hops ch\\_sym\\_udn ch\\_sym\\_sm\\_no ch\\_sym\\_sm ch\\_sym\\_sm\\_nullack\n");
  fprintf(out2_sym_file, "0 a b c d e f\n");
  sprintf(out2_asymin_str, "%d_%d_asymin_all_stat.dat.avg", type, nscamb);
  openFile(out2_asymin_str, out2_asymin_file, "w");
  //fprintf(out2_asymin_file, "hops ch\\_asymin\\_udn ch\\_asymin\\_sm ch\\_asymin\\_sm\\_all\n");
  fprintf(out2_asymin_file, "0 a b c\n");

  for (i=0; i<3; i++) {
    double value[3];
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
