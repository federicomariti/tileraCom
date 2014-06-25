#define __USE_BSD
#include <sys/time.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <getopt.h>

#if HANDLE_DEADLOCK >= 1
# include <unistd.h>
# include <signal.h>
#endif

#include <pthread.h>

#include <tmc/cpus.h>
#include <tmc/task.h>
#include <tmc/udn.h>
#include <arch/atomic.h>
#include <arch/cycle.h>

#include "error_handler.h"
#include "ch_sym_ref_ad1_udn.h"
#include "ch_sym_ref_ad1_sm.h"
#include "ch_sym_ref_ad1_sm_null.h"
#include "ch_sym_ref_ad1_sm_fence.h"
#include "ch_sym_ref_ad1_sm_nullack.h"
#include "ch_sym_ref_ad1_sm_no.h"
#include "ch_asymin_ref_ad1_udn.h"
#include "ch_asymin_ref_ad1_sm_struct.h"
#include "ch_asymin_ref_ad1_sm_struct_b.h"
#include "ch_asymin_ref_ad1_sm_param.h"
#include "ch_asymin_ref_ad1_sm_param_b.h"
#include "ch_asymin_ref_ad1_sm_pthr.h"
#include "ch_asymin_ref_ad1_sm_pthr_b.h"

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#ifndef TEST_TYPE
/**
 * TEST_TYPE macro value:
 * 0: in un test: m scambi, m misurazioni
 * 1: in un test: m scambi, 2m misurazioni
 * 2: in un test: m scambi, 1 misurazione
 */
# define TEST_TYPE 0
#endif

#ifndef TEST_SKIP_FIRST
# define TEST_SKIP_FIRST 10
#endif

#ifndef TEST_VERBOSE
# define TEST_VERBOSE 0
#endif 
#ifndef TEST_DEBUG
# define TEST_DEBUG 0
#endif
#ifndef TEST_RESULT
# define TEST_RESULT 0
#endif
#ifndef HANDLE_DEADLOCK
# define HANDLE_DEADLOCK 0
#endif
#ifndef TEST_CORRECTNESS
# define TEST_CORRECTNESS 0
#endif
#ifndef TEST_PEAK
# define TEST_PEAK 0
#endif

#ifdef METER_SYM_UDN
# define SYMMETRIC
# define METER_STRING "ch_sym_ref_ad1_udn"
# define CH0_IMPL ch_sym_ref_ad1_udn
# define CH1_IMPL ch_sym_ref_ad1_udn
# define CH0_CREATE_ARGS ,0,0
# define CH1_CREATE_ARGS ,1,1
#elif defined METER_SYM_SM_FENCE
# define SYMMETRIC
# define METER_STRING "ch_sym_ref_ad1_sm_fence"
# define CH0_IMPL ch_sym_ref_ad1_sm_fence
# define CH1_IMPL ch_sym_ref_ad1_sm_fence
# define CH0_CREATE_ARGS ,NULL
# define CH1_CREATE_ARGS ,NULL
#elif defined METER_SYM_SM_NULLACK
# define SYMMETRIC
# define METER_STRING "ch_sym_ref_ad1_sm_nullack"
# define CH0_IMPL ch_sym_ref_ad1_sm_nullack
# define CH1_IMPL ch_sym_ref_ad1_sm_nullack
# define CH0_CREATE_ARGS ,NULL
# define CH1_CREATE_ARGS ,NULL
#elif defined METER_SYM_SM_NULL
# define SYMMETRIC
# define METER_STRING "ch_sym_ref_ad1_sm_null"
# define CH0_IMPL ch_sym_ref_ad1_sm_null
# define CH1_IMPL ch_sym_ref_ad1_sm_null
# define CH0_CREATE_ARGS ,NULL
# define CH1_CREATE_ARGS ,NULL
#elif defined METER_SYM_SM_NO
# define SYMMETRIC
# define METER_STRING "ch_sym_ref_ad1_sm_no"
# define CH0_IMPL ch_sym_ref_ad1_sm_no
# define CH1_IMPL ch_sym_ref_ad1_sm_no
# define CH0_CREATE_ARGS ,NULL
# define CH1_CREATE_ARGS ,NULL
#elif defined METER_ASYMIN_UDN
# define ASYMMETRIC
# define SEND_CPU_PARAM_RANK
# define METER_STRING "ch_asymin_ref_ad1_udn"
# define CH0_IMPL ch_asymin_ref_ad1_udn
# define CH1_IMPL ch_asymin_ref_ad1_udn
# define CH0_CREATE_ARGS ,0,0
# define CH1_CREATE_ARGS ,1,1
#elif defined METER_ASYMIN_SM_STRUCT
# define ASYMMETRIC
# define SENDER_INIT
# define METER_STRING "ch_asymin_ref_ad1_sm_struct"
# define CH0_IMPL ch_asymin_ref_ad1_sm_struct
# define CH1_IMPL ch_asymin_ref_ad1_sm_struct
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#elif defined METER_ASYMIN_SM_STRUCT_B
# define ASYMMETRIC
# define SENDER_INIT
# define METER_STRING "ch_asymin_ref_ad1_sm_struct_b"
# define CH0_IMPL ch_asymin_ref_ad1_sm_struct_b
# define CH1_IMPL ch_asymin_ref_ad1_sm_struct_b
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#elif defined METER_ASYMIN_SM_PARAM
# define ASYMMETRIC 
# define SEND_CPU_PARAM_CPU
# define METER_STRING "ch_asymin_ref_ad1_sm_param"
# define CH0_IMPL ch_asymin_ref_ad1_sm_param
# define CH1_IMPL ch_asymin_ref_ad1_sm_param
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#elif defined METER_ASYMIN_SM_PARAM_B
# define ASYMMETRIC 
# define SEND_CPU_PARAM_RANK
# define METER_STRING "ch_asymin_ref_ad1_sm_param_b"
# define CH0_IMPL ch_asymin_ref_ad1_sm_param_b
# define CH1_IMPL ch_asymin_ref_ad1_sm_param_b
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#elif defined METER_ASYMIN_SM_PTHR
# define ASYMMETRIC 
# define METER_STRING "ch_asymin_ref_ad1_sm_pthr"
# define CH0_IMPL ch_asymin_ref_ad1_sm_pthr
# define CH1_IMPL ch_asymin_ref_ad1_sm_pthr
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#elif defined METER_ASYMIN_SM_PTHR_B
# define ASYMMETRIC 
# define METER_STRING "ch_asymin_ref_ad1_sm_pthr_b"
# define CH0_IMPL ch_asymin_ref_ad1_sm_pthr_b
# define CH1_IMPL ch_asymin_ref_ad1_sm_pthr_b
# define CH0_CREATE_ARGS 
# define CH1_CREATE_ARGS
#endif

#define CONCAT(X, Y) X ## Y
#define ch_create(IMPL) CONCAT(IMPL, _create)
#define ch_destroy(IMPL) CONCAT(IMPL, _destroy)
#define ch_send(IMPL) CONCAT(IMPL, _send)
#define ch_receive(IMPL) CONCAT(IMPL, _receive)
#define ch_init_sender(IMPL) CONCAT(IMPL, _init_sender)
#define ch_type(IMPL) CONCAT(IMPL, _t)

#define GET_CLOCK_CYCLE __insn_mfspr(SPR_CYCLE_LOW)

#define prepareStatistics(ARRAY, NEW_VALUE)		\
  (ARRAY)[0] = (NEW_VALUE); (ARRAY)[1] += (ARRAY)[0];	\
	 (ARRAY)[2] += (ARRAY)[0] * (ARRAY)[0];		\
	 (ARRAY)[3] = (ARRAY)[3] < (ARRAY)[0] ?		\
    (ARRAY)[0] : (ARRAY)[3];
#define calcStatistics(AVG, SDEV, ARRAY, NITER)				\
  (AVG) = (double)(ARRAY)[1] / (NITER);					\
	(SDEV) = sqrt((double)(ARRAY)[2] / (NITER) - (AVG) * (AVG)); 
#define printStatistics_format(STAT_NAME, MAX_VALUE_TYPE)		\
  "[STAT] "STAT_NAME":\n[STAT]    %-8s%f\n[STAT]    %-8s%f\n[STAT]    %-8s%" \
  MAX_VALUE_TYPE"\n"
#define printStatistics_values(AVG, SDEV, ARRAY)	\
  "avg", (AVG), "sdev", (SDEV), "max", ARRAY[3]

#define printStatistics_format2(STAT_NAME, MAX_VALUE_TYPE)	\
  ""STAT_NAME" avg sdev max: %-20f %-20f %-20"MAX_VALUE_TYPE"\n"
#define printStatistics_values2(AVG, SDEV, ARRAY)	\
  (AVG), (SDEV), ARRAY[3]

////////////////////////////////////////////////////////////////////////////
// TYPEDEFS
////////////////////////////////////////////////////////////////////////////

typedef struct {
  int		cpu;
  int		rank;
  void *	ch[2];
  int		num_scambi;

} arg_t;

////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////

/* threads */
pthread_t		thread_white,	thread_black;
pthread_barrier_t	computation_start, computation_end;
void *			task_pingpong_white(void *);
void *			task_pingpong_black(void *);
uint_reg_t		a, b;

/* statistics */
/* 0 current, 1 sum, 2 square sum, 3 max value */
uint64_t	Tsend[4] = { 0, 0, 0, 0 };

/* debug */
int *iter_ref = NULL;
int *white_i_ref = NULL;
int *black_i_ref = NULL;

FILE *measure_file = NULL;
char measure_file_string[64];

FILE *measure_file_2 = NULL;
char measure_file_string_2[64];

/* limits */
int			niter = 1;
int			nscambi = 100000;
/* threads */
int			cpu_white,	cpu_black,	cpu_main;

////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////

#if HANDLE_DEADLOCK >= 1
#define deadlock_wait 120
#include <limits.h>
void
alarmHandler(int sig)
{
#if TEST_DEBUG >= 1
    fprintf(stderr, "[DEBUG] iter %d white_iter %d black iter %d \n",
	    *iter_ref, *white_i_ref, *black_i_ref);
#endif
    if (stderr != measure_file)
      if (NULL != (measure_file = fopen(measure_file_string, "a"))) {
	fprintf(measure_file, "DL%d DL%d DL%d DL%d DL%d DL%d DL%d DL%d\n",
		cpu_white, cpu_black, abs(cpu_white%8 - cpu_black%8) + abs(cpu_white/8 - cpu_black/8),
		niter, nscambi, *iter_ref, *white_i_ref, *black_i_ref);
	fclose(measure_file);
      }
    tmc_task_die("deadlock");
}
#endif /* HANDLE_DEADLOCK >= 1 */

#define setOpt(VAR, NAME, HAS_ARG, FLAG, VAL) (VAR).name = NAME; (VAR).has_arg = HAS_ARG; (VAR).flag = FLAG; (VAR).val = VAL;

int main(int argc, char **argv)
{
  /* /\* limits *\/ */
  /* int			niter = 1; */
  /* int			nscambi = 100000; */
  /* /\* threads *\/ */
  /* int			cpu_white,	cpu_black,	cpu_main;  */
  int			cpu_mult_white[2] = { 0, -1 };
  int			cpu_mult_black[2] = { 0, -1 };
  int			white_rank = 0, black_rank = 61;
  void *		ch[2];
  /* statistics */
  struct timeval	start;
  struct timeval	end;
  /* 0 current, 1 sum, 2 square sum, 3 max value */
  uint64_t		result_test[4] = { 0, 0, 0, 0 };
  double		elapsed_test[4] = { 0, 0, 0, 0 };
  double		avg_Tsend[4] = { 0 };
  double		sdev_Tsend[4] = { 0 };
  double		max_Tsend[4] = { 0 };
  /* 0 results, 1 elapsed_test, 2 Tsend, 3 avg_Tsend, 4 sdev_Tsend, 5 max_Tsend */
  double		avg[6];
  double		sdev[6];
  /* others */
  cpu_set_t		dp, udn_hardwall;
  int			i;
  int			retval[2];
  int			num_hops;

  int opt = 0;
  int longopt;
  struct option options[5];
  setOpt(options[0],	"niter",	required_argument,	&longopt,	'n');
  setOpt(options[1],	"nscambi",	required_argument,	&longopt,	'm');
  setOpt(options[2],	"white",	required_argument,	&longopt,	'w');
  setOpt(options[3],	"black",	required_argument,	&longopt,	'b');
  setOpt(options[4],	NULL,		0,			NULL,		0);
  while (longopt || -1 != (opt = getopt_long(argc, argv, "n:m:w:b:", options, NULL))) {
    switch (opt) {
    case 'n':  niter = atoi(optarg); break;
    case 'w':  white_rank = atoi(optarg); break;
    case 'b':  black_rank = atoi(optarg); break;
    case 'm':  nscambi = atoi(optarg); break;
    case 0:    opt=longopt; continue;
    }
    longopt = 0;
  }

  //num_hops = abs(cpu_white%8 - cpu_black%8) + abs(cpu_white/8 - cpu_black/8);

  num_hops = abs(white_rank%8 - black_rank%8) + abs(white_rank/8 - black_rank/8);

  sprintf(measure_file_string, "outs_meter/%d_"METER_STRING".dat", TEST_TYPE);
  if (NULL == (measure_file = fopen(measure_file_string, "a")))
    measure_file = stderr;

  if (niter == 1) {
    sprintf(measure_file_string_2, 
	    "outs_meter/%d_%d_%d_"METER_STRING".dat", 
	    TEST_TYPE, (int)log10(nscambi), num_hops);
    if (NULL == (measure_file_2 = fopen(measure_file_string_2, "a")))
      measure_file_2 = stderr;
  } else {
    measure_file_2 = stderr;
  }

  /* defines cpus */
  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  if (tmc_cpus_count(&dp) < 3) {
    fprintf(stderr,
	    "[ERROR] numero di cpu dataplane disponibili non sufficiente\n");
  }
  ERRHAND(cpu_white = tmc_cpus_find_nth_cpu(&dp, white_rank));
  ERRHAND(cpu_black = tmc_cpus_find_nth_cpu(&dp, black_rank));
  ERRHAND(cpu_main = tmc_cpus_find_nth_cpu(&dp, tmc_cpus_count(&dp)-2));
  cpu_mult_white[0] = cpu_white;
  cpu_mult_black[0] = cpu_black;
  
  /* bind this process to a dataplane cpu */
  ERRHAND(tmc_cpus_set_my_cpu(cpu_main));

#if TEST_VERBOSE >= 1
  printf("[INFO] main: cpu %d niter %d\n", tmc_cpus_get_my_cpu(), niter);
#endif

#if TEST_VERBOSE >= 1
  fprintf(stderr,
	  "-- main task on cpu %d, white task on cpu %d, black task on cpu %d; \n"
	  "-- num. of test iteration %d, num. of measures %d\n",
	  tmc_cpus_get_my_cpu(), cpu_white, cpu_black, niter, nscambi);
#endif

  /* define and initialize udn hardwall */
  tmc_cpus_clear(&udn_hardwall);
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_main));
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_white));
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_black));

  /* init synchronization barriers */
  ERRHAND_NZ(pthread_barrier_init(&computation_start, NULL, 2));
  ERRHAND_NZ(pthread_barrier_init(&computation_end, NULL, 2));

#if HANDLE_DEADLOCK >= 1
  signal(SIGALRM, alarmHandler);
#endif

#if TEST_DEBUG >= 1 || HANDLE_DEADLOCK >= 1
  iter_ref = &i;
#endif  

  for (i=0; i<niter; i++) {
    arg_t arg[2];

    Tsend[1] = 0; /* sum */
    Tsend[2] = 0; /* square sum */
    Tsend[3] = 0; /* max */

#if TEST_DEBUG >= 1
    printf("[DEBUG] main: test %d\n", i);
#endif
    
    /* START TEST i-esimo */
#if HANDLE_DEADLOCK >= 1
    alarm(deadlock_wait);
#endif
    ERRHAND(gettimeofday(&start, NULL));

    /* setup environment */
    ERRHAND(tmc_udn_init(&udn_hardwall));
#ifdef SYMMETRIC
    ERRHAND_NN(ch[0] = ch_create(CH0_IMPL)(cpu_white, cpu_black CH0_CREATE_ARGS));
    ERRHAND_NN(ch[1] = ch_create(CH1_IMPL)(cpu_black, cpu_white CH1_CREATE_ARGS));
#elif defined ASYMMETRIC
    // cpu_black is the receiver of ch0, cpu_white is the only sender of ch0
    ERRHAND_NN(ch[0] = ch_create(CH0_IMPL)(cpu_black, cpu_mult_white CH0_CREATE_ARGS));
    // cpu_white is the receiver of ch1, cpu_black is the only sender of ch1
    ERRHAND_NN(ch[1] = ch_create(CH1_IMPL)(cpu_white, cpu_mult_black CH1_CREATE_ARGS));
#endif
    
    arg[0].cpu = cpu_white;
    arg[0].rank = 0;
    arg[0].ch[0] = ch[0];
    arg[0].ch[1] = ch[1];
    arg[0].num_scambi = nscambi;
    arg[1].cpu = cpu_black;
    arg[1].rank = 0;
    arg[1].ch[0] = ch[0];
    arg[1].ch[1] = ch[1];
    arg[1].num_scambi = nscambi;

    /* start computation */
    ERRHAND_NZ(pthread_create(&thread_white, NULL, task_pingpong_white,
			      (void *)&arg[0]));
    ERRHAND_NZ(pthread_create(&thread_black, NULL, task_pingpong_black,
			      (void *)&arg[1]));

    /* wait end of computation */
    ERRHAND_NZ(pthread_join(thread_white, (void *)retval));
    ERRHAND_NZ(pthread_join(thread_black, (void *)(retval+1)));

    /* destroy environment */
    ch_destroy(CH0_IMPL)((ch_type(CH0_IMPL)**)ch);
    ch_destroy(CH1_IMPL)((ch_type(CH1_IMPL)**)ch+1);
    ERRHAND(tmc_udn_close());

    /* END TEST i-esimo */
    ERRHAND(gettimeofday(&end, NULL));

    /* calcola statistiche sugli scambi eseguiti nel test corrente */
#if TEST_TYPE == 0
    calcStatistics(avg[2], sdev[2], Tsend, nscambi);
#elif TEST_TYPE == 1
    calcStatistics(avg[2], sdev[2], Tsend, 2*nscambi);
#elif TEST_TYPE == 2
    avg[2] = Tsend[1]/(double)(2*nscambi);
    sdev[2] = 0;
#endif

    /* prepara le informazioni per le statistiche sulla suite di test */
    timersub(&end, &start, &start);
    prepareStatistics(elapsed_test, start.tv_sec*1000+start.tv_usec/(double)1000);
    prepareStatistics(result_test, retval[0] + retval[1]);
    prepareStatistics(avg_Tsend, avg[2]);
    prepareStatistics(sdev_Tsend, sdev[2]);
    prepareStatistics(max_Tsend, Tsend[3]);

  } /* for (i=0; i<niter; i++) */

  /* calcola le statistiche per la suite di test eseguita */
  calcStatistics(avg[0], sdev[0], result_test, niter);
  calcStatistics(avg[1], sdev[1], elapsed_test, niter);
  calcStatistics(avg[3], sdev[3], avg_Tsend, niter);
  calcStatistics(avg[4], sdev[4], sdev_Tsend, niter);
  calcStatistics(avg[5], sdev[5], max_Tsend, niter);

  /* stampa i risultati */
  fprintf(stderr,
	  printStatistics_format2("Tsend-avg (cycles)", "f")
	  printStatistics_format2("Tsend-sdev        ", "f")
	  printStatistics_format2("Tsend-max (cycles)", "f")
	  "---\n"
	  printStatistics_format2("Elapsed time (ms)", "f"),
	  printStatistics_values2(avg[3], sdev[3], avg_Tsend),
	  printStatistics_values2(avg[4], sdev[4], sdev_Tsend),
	  printStatistics_values2(avg[5], sdev[5], max_Tsend),
	  printStatistics_values2(avg[1], sdev[1], elapsed_test)
	  );

  if (measure_file != stderr) {
    if (NULL != (measure_file = fopen(measure_file_string, "a")))
      fprintf(measure_file, "%d %d %d %d %d %f %f %f\n",
	      cpu_white, cpu_black, num_hops,
	      niter, nscambi, avg[3], avg[4], avg[5]);
    fclose(measure_file);
  }

  if (measure_file_2 != stderr) {
    fprintf(measure_file_2, "%f %f %f\n", avg[3], avg[4], avg[5]);
    fclose(measure_file_2);
  }
    
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// TASKS
////////////////////////////////////////////////////////////////////////////

void *
task_pingpong_white(void *arg)
{
  arg_t *	Arg = (arg_t *)arg;
  register int	i;
  int		integer;
  int		result = 0;
  int *		received = NULL;

  register int	num_scambi = Arg->num_scambi;
  register void *ch0;		   /* output channel */
  register void *ch1 = Arg->ch[1]; /* input channel */
  register int cpu = Arg->cpu;
  register int rank = Arg->rank;

#if TEST_DEBUG >= 1 || HANDLE_DEADLOCK >= 1
  white_i_ref = &integer;
#endif

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  /*ERRHAND_TILERA*/if (-1 == tmc_udn_activate()) perror("tmc_udn_activate");

#if defined ASYMMETRIC && defined SENDER_INIT
  ch_init_sender(CH0_IMPL)((ch_type(CH0_IMPL)**)Arg->ch);
  if (NULL == Arg->ch[0]) tmc_task_die("sender: init_sender return 0");
#endif /* defined ASYMMETRIC && defined SENDER_INIT */

  ch0 = Arg->ch[0];

  pthread_barrier_wait(&computation_start);

  for (i=0; i<TEST_SKIP_FIRST; i++) {
#if defined SEND_CPU_PARAM_CPU
    ch_send(CH0_IMPL)(ch0, &integer, cpu); //-->
#elif defined SEND_CPU_PARAM_RANK
    ch_send(CH0_IMPL)(ch0, &integer, rank); //-->
#else
    ch_send(CH0_IMPL)(ch0, &integer); //-->
#endif /* SEND_CPU_PARAM_CPU or SEND_CPU_PARAM_RANK or NO THIRD PARAM */
    received = ch_receive(CH1_IMPL)(ch1); //<--
  } 

#if TEST_TYPE == 2
  atomic_compiler_barrier();
  a = GET_CLOCK_CYCLE;
  atomic_compiler_barrier();
#endif /* TEST_TYPE == 2 */

  for (i=0; i<num_scambi; i++) {
    integer = i;

    atomic_compiler_barrier();
#if TEST_TYPE == 0 || TEST_TYPE == 1
    a = GET_CLOCK_CYCLE;
    atomic_compiler_barrier();
#endif /* TEST_TYPE == 0 || TEST_TYPE == 1 */

#if defined SEND_CPU_PARAM_CPU
    ch_send(CH0_IMPL)(ch0, &integer, cpu); //-->
#elif defined SEND_CPU_PARAM_RANK
    ch_send(CH0_IMPL)(ch0, &integer, rank); //-->
#else
    ch_send(CH0_IMPL)(ch0, &integer); //-->
#endif /* SEND_CPU_PARAM_CPU or SEND_CPU_PARAM_RANK or NO THIRD PARAM */

    atomic_compiler_barrier();
    received = ch_receive(CH1_IMPL)(ch1); //<--
    atomic_compiler_barrier();

#if TEST_TYPE == 1
    b = GET_CLOCK_CYCLE;
    atomic_compiler_barrier();
    if (b > a) { prepareStatistics(Tsend, b-a); }
#endif /* TEST_TYPE == 1 */

#if TEST_CORRECTNESS == 1
    if (NULL == received) {
      result ++;
      fprintf(stderr, "[ERROR] white: null received\n");
    } else {
      if (&integer != received) result ++;
    }
#endif /* TEST_CORRECTNESS == 1 */

#if TEST_DEBUG >= 2
    if (i%10000000 == 0) printf("[DEBUG] sender: i = %d\n", i);
#endif /* TEST_DEBUG >= 2 */
    
  } /* for (i=0; i<num_scambi; i++) */

#if TEST_TYPE == 2
  atomic_compiler_barrier();
  b = GET_CLOCK_CYCLE;
  if (b > a) {
    Tsend[1] = b-a;
    /* no sdev, no max */
  }
  atomic_compiler_barrier();
#endif /* TEST_TYPE == 2 */

  ERRHAND_TILERA(tmc_udn_close());
  return (void *)result;
}

void *
task_pingpong_black(void *arg)
{
  arg_t *	Arg = (arg_t *)arg;
  int		i;
  int		result = 0;
  int *		received;

  register int num_scambi = Arg->num_scambi;
  register void *ch0 = Arg->ch[0]; /* input channel */
  register void *ch1;		   /* output channel */
  register int cpu = Arg->cpu;
  register int rank = Arg->rank;

#if TEST_DEBUG >= 1 || HANDLE_DEADLOCK >= 1
  black_i_ref = &i;
#endif
  
  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  /*ERRHAND_TILERA*/if (-1 == tmc_udn_activate()) perror("tmc_udn_activate");
  pthread_barrier_wait(&computation_start);

#if defined ASYMMETRIC && defined SENDER_INIT
  ch_init_sender(CH1_IMPL)((ch_type(CH1_IMPL)**)Arg->ch+1);
  if (NULL == Arg->ch[1]) tmc_task_die("sender: init_sender return 0");
#endif /* defined ASYMMETRIC && defined SENDER_INIT */

  ch1 = Arg->ch[1];

  for (i=0; i<TEST_SKIP_FIRST; i++) {
    received = ch_receive(CH0_IMPL)(ch0); //<--    
#if defined SEND_CPU_PARAM_CPU
    ch_send(CH1_IMPL)(ch1, received, cpu); //-->
#elif defined SEND_CPU_PARAM_RANK
    ch_send(CH1_IMPL)(ch1, received, rank); //-->
#else    
    ch_send(CH1_IMPL)(ch1, received); //-->
#endif
  }
  
  for (i=0; i<num_scambi; i++) {
    atomic_compiler_barrier();
    received = ch_receive(CH0_IMPL)(ch0); //<--

#if TEST_TYPE == 0 || TEST_TYPE == 1
    atomic_compiler_barrier();
    b = GET_CLOCK_CYCLE;
    atomic_compiler_barrier();
    if (b > a) { prepareStatistics(Tsend, b-a); }
#endif /* TEST_TYPE == 0 || TEST_TYPE == 1 */

#if TEST_DEBUG >= 3 || TEST_PEAK > 0
    if (b-a > TEST_PEAK) {
      fprintf(stderr, "i %-20d Tsend %"PRIu64"\n", i, Tsend[0]);
    }
#endif /* TEST_DEBUG >= 3 || TEST_PEAK == 1 */

#if TEST_CORRECTNESS == 1
    if (NULL == received) {
      result ++;
      fprintf(stderr, "[ERROR] black: null received\n");
    }
#endif /* TEST_CORRECTNESS == 1 */

#if TEST_TYPE == 1
    atomic_compiler_barrier();
    a = GET_CLOCK_CYCLE;
#endif /* TEST_TYPE == 1 */

    atomic_compiler_barrier();

#if defined SEND_CPU_PARAM_CPU
    ch_send(CH1_IMPL)(ch1, received, cpu); //-->
#elif defined SEND_CPU_PARAM_RANK
    ch_send(CH1_IMPL)(ch1, received, rank); //-->
#else    
    ch_send(CH1_IMPL)(ch1, received); //-->
#endif
    atomic_compiler_barrier();

#if TEST_DEBUG >= 2
    if (i%10000000 == 0) printf("[DEBUG] receiver: i = %d\n", i);
#endif /* TEST_DEBUG >= 2 */

  } /* for (i=0; i<num_scambi; i++) */

  ERRHAND_TILERA(tmc_udn_close());
  return (void *)result;
}
