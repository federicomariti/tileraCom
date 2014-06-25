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

#include <pthread.h>
#include <unistd.h>
#include <signal.h>

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

////////////////////////////////////////////////////////////////////////////
// DEFINES
////////////////////////////////////////////////////////////////////////////

#ifndef TEST_VERBOSE
# define TEST_VERBOSE 0
#endif 
#ifndef TEST_DEBUG
# define TEST_DEBUG 0
#endif
#ifndef TEST_RESULT
# define TEST_RESULT 0
#endif

#define DO_VERBOSE(LEVEL, ARG) if (TEST_VERBOSE >= LEVEL) { ARG; }
#define DO_DEBUG(ARG) if (TEST_DEBUG) { ARG; }

#ifdef CH0_USE_UDN
# define CH0_IMPL udn
# define CH0_CREATE_ARGS , 0, 0, &udn_hardwall
#endif
#ifdef CH0_USE_SM_NULL
# define CH0_IMPL sm_null
# define CH0_CREATE_ARGS , NULL
#endif
#ifdef CH0_USE_SM_FENCE
# define CH0_IMPL sm_fence
# define CH0_CREATE_ARGS , NULL
#endif

#ifdef CH1_USE_UDN
# define CH1_IMPL udn
# define CH1_CREATE_ARGS , 1, 1, &udn_hardwall
#endif
#ifdef CH1_USE_SM_NULL
# define CH1_IMPL sm_null
# define CH1_CREATE_ARGS , NULL
#endif
#ifdef CH1_USE_SM_FENCE
# define CH1_IMPL sm_fence
# define CH1_CREATE_ARGS , NULL
#endif

/* channel 0 default */
#ifndef CH0_IMPL
# define CH0_IMPL udn
# define CH0_CREATE_ARGS 0, 0, &udn_hardwall
#endif
/* channel 1 default */
#ifndef CH1_IMPL
# define CH1_IMPL udn
# define CH1_CREATE_ARGS 1, 1, &udn_hardwall
#endif


#define CONCAT(X, Y, Z) X ## Y ## Z
#define ch_create(IMPL) CONCAT(ch_sym_ref_ad1_, IMPL, _create)
#define ch_destroy(IMPL) CONCAT(ch_sym_ref_ad1_, IMPL, _destroy)
#define ch_send(IMPL) CONCAT(ch_sym_ref_ad1_, IMPL, _send)
#define ch_receive(IMPL) CONCAT(ch_sym_ref_ad1_, IMPL, _receive)
#define ch_fun(FUN_NAME, IMPL) CONCAT(ch_sym_ref_ad1_, IMPL, _##FUN_NAME)

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
  ""STAT_NAME":\t\t%f\t%f\t%"MAX_VALUE_TYPE"\n"
#define printStatistics_values2(AVG, SDEV, ARRAY)	\
  (AVG), (SDEV), ARRAY[3]

typedef struct {
  int		cpu;
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

/* statistics */
/* 0 current, 1 sum, 2 square sum, 3 max value */
uint64_t	Tscambio[4] = { 0, 0, 0, 0 };

/* deadlock */
int	deadlock_continue = 0;
int	deadlock_timeout = 5;
void	sighand_alrm(int);

////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////

static void
printHelp(void)
{
  printf("Options: -n|--niter pos-int                \t number of test runs \n"
	 );
  printf("Default values: niter = 1"
	 );
}
  
int main(int argc, char **argv)
{
  /* limits */
  int			niter = 1;
  int			nscambi = 100000;
  /* threads */
  int			cpu_white,	cpu_black,	cpu_main;
  int			white_rank = 0, black_rank = 61;
  void *		ch[2];
  /* statistics */
  struct timeval	start;
  struct timeval	end;
  /* 0 current, 1 sum, 2 square sum, 3 max value */
  uint64_t		result_test[4] = { 0, 0, 0, 0 };
  double		elapsed_test[4] = { 0, 0, 0, 0 };
  double		avg_Tscambio[4] = { 0 };
  double		sdev_Tscambio[4] = { 0 };
  double		max_Tscambio[4] = { 0 };

  /* 0 results, 1 elapsed_test, 2 Tscambio, 3 avg_Tscambio, 4 sdev_Tscambio, 5 max_Tscambio */
  double		avg[6];
  double		sdev[6];
  /* others */
  cpu_set_t		dp, udn_hardwall;
  int			i;
  int			retval[2];

  int opt;
  int longopt;
  struct option options[] = {
    { "niter",	required_argument,	&longopt,	'n' },
    { "nscambi",required_argument,	&longopt,	'm' },
    { "white",	required_argument,	&longopt,	'w' },
    { "black",	required_argument,	&longopt,	'b' },
    { NULL,	0,			NULL,		0 }
  };

  while (longopt || -1 != (opt = getopt_long(argc, argv, "n:m:w:b:", options, NULL))) {
    switch (opt) {
    case 'n':
      niter = atoi(optarg);
      break;
    case 'w':
      white_rank = atoi(optarg);
      break;
    case 'b':
      black_rank = atoi(optarg);
      break;
    case 'm':
      nscambi = atoi(optarg);
      break;
    case 0:
      opt=longopt;
      continue;
    }
    longopt =0;
  }    

  signal(SIGALRM, sighand_alrm);

  /* defines cpus */
  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  if (tmc_cpus_count(&dp) < 3)
    fprintf(stderr,
	    "[ERROR] numero di cpu dataplane disponibili non sufficiente\n");
  //ERRHAND(cpu_white = tmc_cpus_find_first_cpu(&dp));
  //ERRHAND(cpu_black = tmc_cpus_find_last_cpu(&dp));
  ERRHAND(cpu_white = tmc_cpus_find_nth_cpu(&dp, white_rank));
  ERRHAND(cpu_black = tmc_cpus_find_nth_cpu(&dp, black_rank));
  ERRHAND(cpu_main = tmc_cpus_find_nth_cpu(&dp, tmc_cpus_count(&dp)-2));
  
  /* bind this process to a dataplane cpu */
  ERRHAND(tmc_cpus_set_my_cpu(cpu_main));

#if TEST_VERBOSE >= 1
  printf("[INFO] main: cpu %d niter %d\n", tmc_cpus_get_my_cpu(), niter);
#endif

  printf("main on cpu %d, white on cpu %d, black on cpu %d, "
	 "num of test iteration %d, num of exchanges %d\n",
	 tmc_cpus_get_my_cpu(), cpu_white, cpu_black, niter, nscambi);  

  /* define ansd initialize udn hardwall */
  tmc_cpus_clear(&udn_hardwall);
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_main));
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_white));
  ERRHAND(tmc_cpus_add_cpu(&udn_hardwall, cpu_black));
  ERRHAND(tmc_udn_init(&dp));

  /* init synchronization barriers */
  ERRHAND_NZ(pthread_barrier_init(&computation_start, NULL, 2));
  ERRHAND_NZ(pthread_barrier_init(&computation_end, NULL, 2));

  for (i=0; i<niter; i++) {
    arg_t arg[2];

    Tscambio[1] = 0;
    Tscambio[2] = 0;
    Tscambio[3] = 0;

    /* START TEST i-esimo */
    ERRHAND(gettimeofday(&start, NULL));

    /* set deadlock alarm */
    alarm(deadlock_timeout);

    /* setup environment */
    ERRHAND_NN(ch[0] = ch_create(CH0_IMPL)(cpu_white, cpu_black CH0_CREATE_ARGS));
    ERRHAND_NN(ch[1] = ch_create(CH1_IMPL)(cpu_black, cpu_white CH1_CREATE_ARGS));
    
    arg[0].cpu = cpu_white;
    arg[0].ch[0] = ch[0];
    arg[0].ch[1] = ch[1];
    arg[0].num_scambi = nscambi;
    arg[1].cpu = cpu_black;
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
    ch_destroy(CH0_IMPL)(ch);
    ch_destroy(CH1_IMPL)(ch+1);

    /* END TEST i-esimo */
    ERRHAND(gettimeofday(&end, NULL));

    /* statistiche sugli scambi eseguiti nel test corrente */
    calcStatistics(avg[2], sdev[2], Tscambio, nscambi);

    timersub(&end, &start, &start);
    prepareStatistics(elapsed_test, start.tv_sec*1000+start.tv_usec/(double)1000);
    prepareStatistics(result_test, retval[0] + retval[1]);
    prepareStatistics(avg_Tscambio, avg[2]);
    prepareStatistics(sdev_Tscambio, sdev[2]);
    prepareStatistics(max_Tscambio, Tscambio[3]);

#if TEST_VERBOSE == 0
    //fprintf(stderr, "%d:%f:%f:%f:(%f);", i, avg[2]/2, sdev[2], (double)Tscambio[3]/2, avg_Tscambio[2]);
#elif TEST_VERBOSE >= 2
    fprintf(stderr, printStatistics_format("Tscambio (cycles)", PRIu64)
	    "[STAT] Tsend (cycles):\n[STAT]    %f\n",
	    printStatistics_values(avg[2], sdev[2], Tscambio),
	    avg[2]/(double)2
	    );
#endif /* TEST_VERBOSE == 0 */

    deadlock_continue = 0;
  } /* for (i=0; i<niter; i++) */


  calcStatistics(avg[0], sdev[0], result_test, niter);
  calcStatistics(avg[1], sdev[1], elapsed_test, niter);
  calcStatistics(avg[3], sdev[3], avg_Tscambio, niter);
  calcStatistics(avg[4], sdev[4], sdev_Tscambio, niter);
  calcStatistics(avg[5], sdev[5], max_Tscambio, niter);

  /*
    fprintf(stderr,
	  printStatistics_format("Tscambio avg (cycles)", "f")
	  printStatistics_format("Tscambio sdev (cycles)", "f")
	  "[STAT] Tscambio max (cycles):\n[STAT]    %f\n",
	  printStatistics_values(avg[3], sdev[3], avg_Tscambio),
	  printStatistics_values(avg[4], sdev[4], sdev_Tscambio),
	  maxmax_Tscambio
	  );
  */
  /*
  fprintf(stderr,
	  printStatistics_format2("Tscambio avg (cycles)", "f")
	  printStatistics_format2("Tscambio sdev        ", "f")
	  printStatistics_format2("Tscambio max (cycles)", "f"),
	  printStatistics_values2(avg[3], sdev[3], avg_Tscambio),
	  printStatistics_values2(avg[4], sdev[4], sdev_Tscambio),
	  printStatistics_values2(avg[5], sdev[5], max_Tscambio)
	  );
Tscambio avg (cycles):		110.491957	0.258812	111.400840
Tscambio sdev        :		118.790573	63.409627	306.372066
Tscambio max (cycles):		34756.240000	18675.977854	80419.000000
  */
  
  fprintf(stderr,
	  "%-20s %-20s %-20s\n"
	  "%-20f %-20f %-20f\n",
	  "avg", "sdev", "max", avg[3], avg[4], avg[5]);

  fprintf(stderr,
	  "\n\n"
	  "              %-20s %-20s %-20s\n"
	  "Tscambio-avg: %-20f %-20f %-20f\n"
	  "Tscambio-dev: %-20f %-20f %-20f\n"
	  "Tscambio-max: %-20f %-20f %-20f\n",
	  "avg", "sdev", "max",
	  avg[3], avg[4], avg[5],
	  sdev[3], sdev[4], sdev[5],
	  avg_Tscambio[3], sdev_Tscambio[3], max_Tscambio[3]
	  );


#if TEST_VERBOSE == 0
#else
#endif /* TEST_VERBOSE == 0 */

  return 0;
}

////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////

void
sighand_alrm(int sig)
{
  if (deadlock_continue) {
    fprintf(stderr, "[ERROR] Can't stop all tasks. Killing the application.\n");
    tmc_task_terminate_app();
  }
  fprintf(stderr, "[ERROR] Deadlock ...\n");
  pthread_cancel(thread_white);
  pthread_cancel(thread_black);
  deadlock_continue = 1;
  alarm(deadlock_timeout);
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

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(Arg->cpu));
  /*ERRHAND_TILERA*/(tmc_udn_activate());
  pthread_barrier_wait(&computation_start);

#if TEST_VERBOSE >= 1
  printf("[INFO] white: cpu %d\n", tmc_cpus_get_my_cpu());
#endif
  
  for (i=0; i<Arg->num_scambi; i++) {
    int *	received = NULL;
    uint_reg_t	a, b;
    integer = i;

    a = GET_CLOCK_CYCLE;
    atomic_compiler_barrier();
    ch_send(CH0_IMPL)(Arg->ch[0], &integer);
    received = ch_receive(CH1_IMPL)(Arg->ch[1]);
    atomic_compiler_barrier();
    b = GET_CLOCK_CYCLE;

    if (b>a) { prepareStatistics(Tscambio, b-a); }
    else {
      //fprintf(stderr, "\n\n>>> %u %u \n\n", a, b);
    }

#if TEST_DEBUG >= 1
    if (b-a > 700) {
      fprintf(stderr, "i %-20d Tscambio %"PRIu64"\n", i, Tscambio[0]);      
    }
#endif

    if (NULL == received) {
      result ++;
      fprintf(stderr, "[ERROR] white: null received\n");
    } else {
      if (i != *received) result ++;
    }
  }

  ERRHAND_TILERA(tmc_udn_close());
  return (void *)result;
}

void *
task_pingpong_black(void *arg)
{
  arg_t *	Arg = (arg_t *)arg;
  register int	i;
  int		result = 0;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(Arg->cpu));
  /*ERRHAND_TILERA*/(tmc_udn_activate());
  pthread_barrier_wait(&computation_start);

#if TEST_VERBOSE >= 1  
  printf("[INFO] black: %d\n", tmc_cpus_get_my_cpu());
#endif

  for (i=0; i<Arg->num_scambi; i++) {
    int * received;

    received = ch_receive(CH0_IMPL)(Arg->ch[0]);
    ch_send(CH1_IMPL)(Arg->ch[1], received);

    if (NULL == received) {
      result ++;
      fprintf(stderr, "[ERROR] black: null received\n");
    }
  }

  ERRHAND_TILERA(tmc_udn_close());
  return (void *)result;
}
