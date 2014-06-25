/**
 * Tests di ch_sym_ref_sm e ch_sym_ref_sm_no
 * implementazione di ch_sym_ref_sm.h, ovvero tests su
 * canali simmetrici con grado di asincronia al piu` ~100 di tipo
 * riferimento implementati con memoria consivisa.
 *
 * Test Suite 1: Lo scopo e` quello di verificare il corretto funzionamento
 *	di invio e ricezione con un numero di messaggi scambiati
 *	inferiore al grado di asincronia del canale.
 *
 *	Vengono inviati i riferimenti a diversi tipi di oggetti:
 *	intero, coppia di interi, array di interi.
 *  
 * Test Suite 2: Lo scopo e` quello di verificare il corretto funzionamento
 *	di invio e ricezione durante lo scambi di un elevato numero di
 *	messaggi (molto superiore al grado di asincronia del canale).
 *
 *	Vengono inviati gli elementi di un array di interi di
 *	dimensione elevata.
 * 
 * Macro:
 * - TEST_VERBOSE stampa informazioni sui test in corso
 * - CHANNEL_SIMMETRIC_REF__TEST effettua test sui dati privati del
 *     supporto di ch_sym_ref_sm.h
 * - CHANNEL_SIMMETRIC_REF__DEBUG effettua stampe di debug durante
 *     l'esecuzioni delle funzioni definite in
 *     ch_sym_ref_sm.h  
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <tmc/cpus.h>
#include <tmc/task.h>

#include "error_handler.h"
#include "ch_sym_ref_sm.h"

#define K 10
#define N 65536
#define M 16

#ifndef TEST_VERBOSE
# define TEST_VERBOSE 0
#endif 
#define PRINT(ARG) if (TEST_VERBOSE) { ARG; }

#ifdef TAKE_TIME_HEAVY
# define GET_CLOCK_CYCLE get_cycle_count()
#else
# define GET_CLOCK_CYCLE __insn_mfspr(SPR_CYCLE_LOW)
#endif

typedef struct { int x, y; } pair_int_t;

pthread_t			thread_producer;
pthread_t			thread_consumer;
pthread_barrier_t		computation_end;
ch_sym_ref_sm_t *	ch;

void *	task_producer(void *arg);
void *	task_consumer(void *arg);

int	deadlock_continue = 0;
int	deadlock_timeout = 3;
int	cpu_snd;
int	cpu_rcv;
int	niter = 1;
int	cur_suite;
int	iter_producer;
int	iter_consumer;

#include <stdint.h>
#include <inttypes.h>
#include <sys/time.h>
#ifndef __USE_BSD
# define __USE_BSD
#endif

struct timeval	start;
struct timeval	end;

/* 0 current, 1 sum, 2 square sum, 3 max value */
uint64_t	result_suite[4] = { 0, 0, 0, 0 };
double		time_suite[4] = { 0, 0, 0, 0 };
uint64_t	call_send[4] = { 0, 0, 0, 0 };
uint64_t	call_recv[4]= { 0, 0, 0, 0 };
uint64_t	send[4]= { 0, 0, 0, 0 };

int		call_send_cur_i, call_recv_cur_i, send_cur_i;
uint64_t	call_send_cur[MAX_ASYNCH_DEGREE];
uint64_t	call_recv_cur[MAX_ASYNCH_DEGREE];
uint64_t	send_cur[MAX_ASYNCH_DEGREE];

/* 0 results, 1 times, 2 Tcall_send, 3 Tcall_recv, 4 Tsend */
double		avg[5];
double		sdev[5];

#define prepareStatistics(ARRAY, NEW_VALUE)			\
  (ARRAY)[0] = (NEW_VALUE); (ARRAY)[1] += (ARRAY)[0];		\
  (ARRAY)[2] += (ARRAY)[0] * (ARRAY)[0];			\
  (ARRAY)[3] = (ARRAY)[3] < (ARRAY)[0] ?			\
    (ARRAY)[0] : (ARRAY)[3];

#define calcStatistics(AVG, SDEV, ARRAY, NITER)	\
  (AVG) = (double)(ARRAY)[1] / (NITER);				\
	(SDEV) = sqrt((double)(ARRAY)[2] / (NITER) - (AVG) * (AVG)); 

#define printStatistics_format(STAT_NAME, MAX_VALUE_TYPE)		\
  STAT_NAME":\n    %-8s%f\n    %-8s%f\n    %-8s%"MAX_VALUE_TYPE"\n"
#define printStatistics_values(AVG, SDEV, ARRAY)	\
  "avg", (AVG), "sdev", (SDEV), "max", ARRAY[3]
  

void
sighand_alrm(int sig)
{
  printf("iter test %d, iter_producer %d iter_consumer %d\n",
	 cur_suite, iter_producer, iter_consumer);
  if (deadlock_continue) {
    fprintf(stderr, "[ERROR] Can't stop all tasks. "
	    "Killing the application.\n");
    tmc_task_terminate_app();
  }
  printf("[ERROR] Deadlock ...\n");
  //pthread_cancel(thread_producer);
  //pthread_cancel(thread_consumer);
  deadlock_continue = 1;
  alarm(deadlock_timeout);
}

int main(int argc, char **argv)
{
  cpu_set_t dp;
  int retval[2];

  if (argc > 1)
    niter = atoi(argv[1]);

  signal(SIGALRM, sighand_alrm);
  
  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));  
  ERRHAND(cpu_snd = tmc_cpus_find_first_cpu(&dp));
  ERRHAND(cpu_rcv = tmc_cpus_find_last_cpu(&dp));
  
  ERRHAND_NZ(pthread_barrier_init(&computation_end, NULL, 2));

  for (cur_suite=0; cur_suite<niter; cur_suite++) {
    PRINT(fprintf(stderr, "[INFO] start test suite %d\n", cur_suite));
    /* --> init shared objects */
    ERRHAND_NN(ch = ch_sym_ref_sm_create(M, cpu_snd, cpu_rcv, NULL));
    /* set deadlock alarm */
    alarm(deadlock_timeout);

    /* start */
    ERRHAND(gettimeofday(&start, NULL));
    ERRHAND_NZ
      (pthread_create(&thread_producer, NULL, task_producer, (void *)cpu_snd));
    ERRHAND_NZ
      (pthread_create(&thread_consumer, NULL, task_consumer, (void *)cpu_rcv));
    /* wait end */
    ERRHAND_NZ(pthread_join(thread_producer, (void *)retval));
    ERRHAND_NZ(pthread_join(thread_consumer, (void *)(retval+1)));
    /* end */
    ERRHAND(gettimeofday(&end, NULL)); 

    timersub(&end, &start, &start);
    prepareStatistics(time_suite, start.tv_sec*1000+start.tv_usec/(double)1000);
    prepareStatistics(result_suite, retval[0] + retval[1]);
    PRINT(fprintf(stderr, "[INFO] end test suite %d: producer %d consumer %d\n",
		  cur_suite, retval[0], retval[1]));
    /* --> destroy shared object */
    ch_sym_ref_sm_destroy(&ch);
    deadlock_continue = 0;
  }

  calcStatistics(avg[0], sdev[0], result_suite, niter);
  calcStatistics(avg[1], sdev[1], time_suite, niter);  

  printf(printStatistics_format("return values", PRIu64)
	 printStatistics_format("elapsed time", "f")
	 printStatistics_format("Tcall-send", PRIu64)
	 printStatistics_format("Tcall-recv", PRIu64)
	 printStatistics_format("Tsend", PRIu64),
	 printStatistics_values(avg[0], sdev[0], result_suite),
	 printStatistics_values(avg[1], sdev[1], time_suite),
	 printStatistics_values(avg[2], sdev[2], call_send),
	 printStatistics_values(avg[3], sdev[3], call_recv),
	 printStatistics_values(avg[4], sdev[4], send)
	 );

  return result_suite[1];
}

void *
task_producer(void *arg)
{
  int		i;
  int		int_value = 123;
  pair_int_t	pair_value = { 123, 456 };
  int		a[K];
  int		b[N];

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu_snd));

  for (i=0; i<K; i++) a[i] = i;
  for (i=0; i<N; i++) b[i] = i;
  /* */
  ch_sym_ref_sm_send(ch, &int_value);
  ch_sym_ref_sm_send(ch, &pair_value);
  ch_sym_ref_sm_send(ch, a);
  (void)pthread_barrier_wait(&computation_end);
  /* */
  for (iter_producer=0; iter_producer<N; iter_producer++)
    ch_sym_ref_sm_send(ch, b+iter_producer);
  (void)pthread_barrier_wait(&computation_end);

  return (void *)0;
}

void *
task_consumer(void * arg)
{
  int		i;
  int		result[4] = { 0 };
  int *		int_ref = NULL;
  pair_int_t *	pair_ref = NULL;
  int (*	array_int_ref)[K] = NULL;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu_rcv));
  /* */
  int_ref = ch_sym_ref_sm_receive(ch);
  if (123 != *int_ref) { result[0] ++; }
  pair_ref = ch_sym_ref_sm_receive(ch);
  if (123 != (pair_ref)->x || 456 != (pair_ref)->y) { result[1] ++; }
  array_int_ref = ch_sym_ref_sm_receive(ch);
  for (i=0; i<K; i++)
    if (i != (*array_int_ref)[i]) { result[2] ++; }
  PRINT(fprintf(stderr, "[INFO] test1 end with results %d %d %d\n", result[0],
		result[1], result[2]));
  (void)pthread_barrier_wait(&computation_end);
  /* */
  for (iter_consumer=0; iter_consumer<N; iter_consumer++) {
    int_ref = ch_sym_ref_sm_receive(ch);
    if (iter_consumer != *int_ref) { result[3] ++; }
  }
  PRINT(fprintf(stderr, "[INFO] test2 end with result %d\n", result[3]));
  (void)pthread_barrier_wait(&computation_end);

  return (void *)(result[0] + result[1] + result[2] + result[3]);
}
  

void *
task_consumer_(void *arg)
{
  int		i;
  int		result[4] = { 0 };
  int *		int_ref = NULL;
  pair_int_t *	pair_ref = NULL;
  int (*	array_int_ref)[K] = NULL;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu_rcv));

  int_ref = ch_sym_ref_sm_receive(ch);
  if (123 != *int_ref) {
    result[0] ++; printf("[ERROR-0] %d\n", *int_ref); }
  pair_ref = ch_sym_ref_sm_receive(ch);
  if (123 != pair_ref->x || 456 != pair_ref->y) {
    result[1] ++; printf("[ERROR-1] %d %d\n", pair_ref->x, pair_ref->y); }
  array_int_ref = ch_sym_ref_sm_receive(ch);
  for (i=0; i<K; i++)
    if (i != (*array_int_ref)[i]) {
      result[2] ++; printf("[ERROR-2] %d\n", (*array_int_ref)[i]); }
  PRINT(fprintf(stderr, "[INFO] test1a %d test1b %d test1c %d\n", result[0],
		result[1], result[2]));
  (void)pthread_barrier_wait(&computation_end);
  for (i=0; i<N; i++) {
    int_ref = ch_sym_ref_sm_receive(ch);
    if (i != *int_ref) result[3] ++;
  }
  PRINT(fprintf(stderr, "[INFO] test2 %d\n", result[3]));
  (void)pthread_barrier_wait(&computation_end);

  return (void *)(result[0] + result[1] + result[2] + result[3]); 
}
