#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include "error_handler.h"

#include <tmc/cpus.h>
#include <tmc/task.h>
#include <tmc/alloc.h>
#include <arch/cycle.h>
#include <arch/atomic.h>

#define get_clock_cycle __insn_mfspr(SPR_CYCLE_LOW)
#define CYCLE_LENGTH_MILLI 0.0000011566265
#define CYCLE_LENGTH_MICRO 0.0011566265
#define CYCLE_LENGTH_NANO 1.1566265
#define DEFAULT_K 1000

/* data="56 168 280" ; for i in $data ; do sed -n '4p' outs_benchmark_suite/nogatherInt_00_500_181_${i}.dat.avg | cut -d " " -f 2 ; done
46,560252 ms --/500--> 0,093120504 ms --/1,1566265--> 8,051043616933e-02 * 10^6 cycle --> 80510,43616932519 cycle
419.219641 ms --> etc 
1172.976266 ms --> etc


data="56 168 280" ; for i in $data ; do sed -n '4p' outs_benchmark_suite/nogatherInt_00_500_181_${i}_calctime.dbg.avg | cut -d " " -f 2 ; done
80291.562400 cycle
724265.796400 cycle
2027286.886000 cycle


make run_pci_benchmark_sequential
avg sdev max min 74351.900000 -nan 85367 74324
avg sdev max min 733250.424000 -nan 741347 733072
avg sdev max min 2040469.784000 -nan 2056750 2039760
*/

#define NUM_ITERATIONS 500 

#define max(x,y) (x) < (y) ? (y) : (x)
#define min(x, y) (x) > (y) ? (y) : (x)

void oneAllocation_single()
{
  int num_iterations = 500;
  int h, i, j;
  int *A, *B, *C;
  int M[] = { 56, 168, 280, -1 }, * Mcur = M;
  uint_reg_t a, b;
  uint64_t calc_sum = 0, calc_sqsum = 0;
  uint_reg_t calc_max = 0, calc_min = UINT_MAX;
  double avg, sdev, sdev2;
  cpu_set_t dp;
  tmc_alloc_t alloc = TMC_ALLOC_INIT;

  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(tmc_cpus_find_last_cpu(&dp)));

  FILE *output_file;
  ERRHAND_NN(output_file = fopen("benchmark_sequential_data/calculateTime.dat", "a"));

  while (-1 != *Mcur) {
    ERRHAND_TILERA_NN(A = tmc_alloc_map(&alloc, (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(B = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(C = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));

    for (i=0; i<*Mcur; i++)
      for (j=0; j<*Mcur; j++)
	*(A+i*(*Mcur)+j) = (*Mcur)*i + j;
    for (i=0; i<*Mcur; i++) 
      B[i] = i;    

    calc_sum = 0; calc_sqsum = 0; calc_max = 0; calc_min = UINT_MAX;

    atomic_compiler_barrier();
    a = get_clock_cycle;
    atomic_compiler_barrier();
    for (i=0; i<*Mcur; i++) {
      C[i] = 0;
      for (j=0; j<*Mcur; j++)
	C[i] += *(A+i*(*Mcur)+j) * B[j];
    }
    atomic_compiler_barrier();
    b = get_clock_cycle;
    atomic_compiler_barrier();

    a = b-a;

    calc_sum += a;
    calc_sqsum += a*a;
    calc_max = max(calc_max, a);
    calc_min = min(calc_min, a);

    avg = calc_sum;
    sdev = 0;
    sdev2 = 0;

    printf("avg sdev max min %f %f %f %lu %lu\n", avg, sdev, sdev2, calc_max, calc_min);

    fprintf(output_file, "%f ", avg);

    ERRHAND_TILERA(tmc_alloc_unmap(A, (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(B, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(C, (*Mcur) * sizeof(int)));

    Mcur++;
  }
  fprintf(output_file, "\n");
  fclose(output_file);
}


void oneAllocation_iterated()
{
  int num_iterations = 500;
  int h, i, j;
  int *A, *B, *C;
  int M[] = { 56, 168, 280, -1 }, * Mcur = M;
  uint_reg_t a, b;
  uint64_t calc_sum = 0, calc_sqsum = 0;
  uint_reg_t calc_max = 0, calc_min = UINT_MAX;
  double avg, sdev, sdev2;
  cpu_set_t dp;
  tmc_alloc_t alloc = TMC_ALLOC_INIT;

  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(tmc_cpus_find_last_cpu(&dp)));

  while (-1 != *Mcur) {
    ERRHAND_TILERA_NN(A = tmc_alloc_map(&alloc, (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(B = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(C = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));

    for (i=0; i<*Mcur; i++)
      for (j=0; j<*Mcur; j++)
	*(A+i*(*Mcur)+j) = (*Mcur)*i + j;
    for (i=0; i<*Mcur; i++) 
      B[i] = i;    

    calc_sum = 0; calc_sqsum = 0; calc_max = 0; calc_min = UINT_MAX;

    for (h=0; h<NUM_ITERATIONS; h++) {
      atomic_compiler_barrier();
      a = get_clock_cycle;
      atomic_compiler_barrier();
      for (i=0; i<*Mcur; i++) {
	C[i] = 0;
	for (j=0; j<*Mcur; j++)
	  C[i] += *(A+i*(*Mcur)+j) * B[j];
      }
      atomic_compiler_barrier();
      b = get_clock_cycle;
      atomic_compiler_barrier();

      a = b-a;
      //printf("%lu ", a);
      calc_sum += a;
      calc_sqsum += a*a;
      calc_max = max(calc_max, a);
      calc_min = min(calc_min, a);
    }

    avg = calc_sum / (double)NUM_ITERATIONS;
    sdev = sqrt(calc_sqsum / (double)NUM_ITERATIONS - avg * avg);
    sdev2 = (1.0/NUM_ITERATIONS) * sqrt(NUM_ITERATIONS * calc_sqsum - calc_sum * calc_sum);

    printf("avg sdev max min %f %f %f %lu %lu\n", avg, sdev, sdev2, calc_max, calc_min);

    ERRHAND_TILERA(tmc_alloc_unmap(A, (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(B, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(C, (*Mcur) * sizeof(int)));

    Mcur++;
  }
}

void contiguousSet()
{
  int num_iterations = 500;
  int h, i, j;
  int *A, *Abase, *B, *C, *Cbase;
  int M[] = { 56, 168, 280, -1 }, * Mcur = M;
  uint_reg_t a, b;
  uint64_t calc_sum = 0, calc_sqsum = 0;
  uint_reg_t calc_max = 0, calc_min = UINT_MAX;
  double avg, sdev, sdev2;
  cpu_set_t dp;
  tmc_alloc_t alloc = TMC_ALLOC_INIT;

  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(tmc_cpus_find_last_cpu(&dp)));

  while (-1 != *Mcur) {
    ERRHAND_TILERA_NN(A = tmc_alloc_map(&alloc, NUM_ITERATIONS * (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(B = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(C = tmc_alloc_map(&alloc, NUM_ITERATIONS * (*Mcur) * sizeof(int)));
    Abase = A;
    Cbase = C;

    for (i=0; i<*Mcur; i++)
      for (j=0; j<*Mcur; j++)
	*(A+i*(*Mcur)+j) = (*Mcur)*i + j;
    for (i=0; i<*Mcur; i++) 
      B[i] = i;    

    calc_sum = 0; calc_sqsum = 0; calc_max = 0; calc_min = UINT_MAX;

    for (h=0; h<NUM_ITERATIONS; h++) {
      atomic_compiler_barrier();
      a = get_clock_cycle;
      atomic_compiler_barrier();
      for (i=0; i<*Mcur; i++) {
	C[i] = 0;
	for (j=0; j<*Mcur; j++)
	  C[i] += *(A+i*(*Mcur)+j) * B[j];
      }
      atomic_compiler_barrier();
      b = get_clock_cycle;
      atomic_compiler_barrier();

      A += (*Mcur) * (*Mcur);
      C += (*Mcur);

      a = b-a;
      //printf("%lu ", a);
      calc_sum += a;
      calc_sqsum += a*a;
      calc_max = max(calc_max, a);
      calc_min = min(calc_min, a);
    }

    avg = calc_sum / (double)NUM_ITERATIONS;
    sdev = sqrt(calc_sqsum / (double)NUM_ITERATIONS - avg * avg);
    sdev2 = (1.0/NUM_ITERATIONS) * sqrt(NUM_ITERATIONS * calc_sqsum - calc_sum * calc_sum);

    printf("avg sdev max min %f %f %f %lu %lu\n", avg, sdev, sdev2, calc_max, calc_min);

    ERRHAND_TILERA(tmc_alloc_unmap(Abase, NUM_ITERATIONS * (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(B, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(Cbase, NUM_ITERATIONS * (*Mcur) * sizeof(int)));

    Mcur++;
  }
}

void disjointSet()
{
  int num_iterations = 500;
  int h, i, j;
  int *A, *Abase, *B, *C, *Cbase;
  int M[] = { 56, 168, 280, -1 }, * Mcur = M;
  uint_reg_t a, b;
  uint64_t calc_sum = 0, calc_sqsum = 0;
  uint_reg_t calc_max = 0, calc_min = UINT_MAX;
  double avg, sdev, sdev2;
  cpu_set_t dp;
  tmc_alloc_t alloc = TMC_ALLOC_INIT;

  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(tmc_cpus_find_last_cpu(&dp)));

  while (-1 != *Mcur) {
    ERRHAND_TILERA_NN(A = tmc_alloc_map(&alloc, NUM_ITERATIONS * (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(B = tmc_alloc_map(&alloc, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA_NN(C = tmc_alloc_map(&alloc, NUM_ITERATIONS * (*Mcur) * sizeof(int)));
    Abase = A;
    Cbase = C;

    for (i=0; i<*Mcur; i++)
      for (j=0; j<*Mcur; j++)
	*(A+i*(*Mcur)+j) = (*Mcur)*i + j;
    for (i=0; i<*Mcur; i++) 
      B[i] = i;    

    calc_sum = 0; calc_sqsum = 0; calc_max = 0; calc_min = UINT_MAX;

    for (h=0; h<NUM_ITERATIONS; h++) {
      atomic_compiler_barrier();
      a = get_clock_cycle;
      atomic_compiler_barrier();
      for (i=0; i<*Mcur; i++) {
	C[i] = 0;
	for (j=0; j<*Mcur; j++)
	  C[i] += *(A+i*(*Mcur)+j) * B[j];
      }
      atomic_compiler_barrier();
      b = get_clock_cycle;
      atomic_compiler_barrier();

      A += (*Mcur) * (*Mcur);
      C += (*Mcur);

      a = b-a;
      //printf("%lu ", a);
      calc_sum += a;
      calc_sqsum += a*a;
      calc_max = max(calc_max, a);
      calc_min = min(calc_min, a);
    }

    avg = calc_sum / (double)NUM_ITERATIONS;
    sdev = sqrt(calc_sqsum / (double)NUM_ITERATIONS - avg * avg);
    sdev2 = (1.0/NUM_ITERATIONS) * sqrt(NUM_ITERATIONS * calc_sqsum - calc_sum * calc_sum);

    printf("avg sdev max min %f %f %f %lu %lu\n", avg, sdev, sdev2, calc_max, calc_min);

    ERRHAND_TILERA(tmc_alloc_unmap(Abase, NUM_ITERATIONS * (*Mcur) * (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(B, (*Mcur) * sizeof(int)));
    ERRHAND_TILERA(tmc_alloc_unmap(Cbase, NUM_ITERATIONS * (*Mcur) * sizeof(int)));

    Mcur++;
  }
}

int main() 
{
  oneAllocation_single();
  return 0;
}


