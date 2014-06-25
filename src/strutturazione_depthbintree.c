/**
 * multicast implementata con pipeline in workers
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h> 
#include <math.h>
#include <tmc/cpus.h>
#include <tmc/udn.h>
#include <tmc/task.h>

#include "error_handler.h"
#include "ch_sym_ref_ad1_udn.h"
#include "ch_asymin_ref_ad1_udn.h"

////////////////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////////////////

//#define PIPELINE
#define DEPTH_BINTREE

#define get_clock_cycle __insn_mfspr(SPR_CYCLE_LOW)
#define NUM_CPU 60
#define DEFAULT_N 16
#define DEFAULT_K 10
#define DEFAULT_INTERARRIVAL 100
#define worker_computation_latency 1

typedef struct {
  int cpu, rank;
  void *ch_in, *ch_out, *ch_out_left, *ch_out_right;
} worker_arg_t;

typedef struct {
  int cpu;
  void *ch_out;
  int interarrival;
} gen_arg_t;

typedef struct {
  int cpu;
  void *ch_in;
} col_arg_t;

////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////

void *task_generator(void *arg);
void *task_worker(void *arg);
void *task_collector(void *arg);
int worker_cpu_mapping_sequential(worker_arg_t *wor_arg, int parallDegree,
				  const cpu_set_t *dataplanes);
void create_pipeline(worker_arg_t *array, int length, gen_arg_t *generator);
void create_depth_tree(worker_arg_t *node, int length, int height, int node_index,
		       int parent_index, gen_arg_t *generator);
void test_create_depth_tree(worker_arg_t *array, int size);

int parallDegree = DEFAULT_N;
int streamSizeK = DEFAULT_K;
void *ch_results;
pthread_barrier_t start_computation;
pthread_barrier_t end_computation;

////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////

int main()
{
  pthread_t th[NUM_CPU];
  int main_cpu;
  int gen_cpu = 0;
  int col_cpu = 1;
  cpu_set_t dp;
  int i;
  gen_arg_t gen_arg;
  col_arg_t col_arg;
  worker_arg_t wor_arg[NUM_CPU];
  int worker_cpus[NUM_CPU];
  int interarrival = DEFAULT_INTERARRIVAL;

  memset(&gen_arg, 0, sizeof(gen_arg));
  memset(&col_arg, 0, sizeof(col_arg));
  memset(&wor_arg, 0, sizeof(wor_arg));

  /* setup udn hardwall */
  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  ERRHAND(tmc_udn_init(&dp));
  /* set my cpu (dataplane) */
  ERRHAND(main_cpu = tmc_cpus_find_last_cpu(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(main_cpu));
  tmc_cpus_remove_cpu(&dp, main_cpu);

  /* setting task args: cpus */
  ERRHAND(gen_arg.cpu = tmc_cpus_find_nth_cpu(&dp, gen_cpu));
  ERRHAND(col_arg.cpu = tmc_cpus_find_nth_cpu(&dp, col_cpu));
  tmc_cpus_remove_cpu(&dp, gen_arg.cpu);
  tmc_cpus_remove_cpu(&dp, col_arg.cpu);
  worker_cpu_mapping_sequential(wor_arg, parallDegree, &dp);
  
  /* setting task args: channels */
  gen_arg.ch_out = ch_sym_ref_ad1_udn_create(gen_arg.cpu, wor_arg[0].cpu, 1, 0);
#ifdef PIPELINE
  create_pipeline(wor_arg, parallDegree, &gen_arg);
#elif defined DEPTH_BINTREE
  create_depth_tree(wor_arg, parallDegree, log2(parallDegree), 0, -1, &gen_arg);
  test_create_depth_tree(wor_arg, parallDegree);
#endif
  for (i=0; i<parallDegree; i++)
    worker_cpus[i] = wor_arg[i].cpu;
  worker_cpus[parallDegree] = -1;
  ch_results = ch_asymin_ref_ad1_udn_create(col_arg.cpu, worker_cpus, 0, 3);
  col_arg.ch_in = ch_results;

  /* setting task args: others */
  gen_arg.interarrival = interarrival;
  
  /* start computation */
  ERRHAND_NZ(pthread_barrier_init(&start_computation, NULL, parallDegree+2));
  ERRHAND_NZ(pthread_barrier_init(&end_computation, NULL, parallDegree+2));
  ERRHAND_NZ(pthread_create(th, NULL, task_generator, (void *)&gen_arg));
  ERRHAND_NZ(pthread_create(th+1, NULL, task_collector, (void *)&col_arg));
  for (i=0; i<parallDegree; i++)
    ERRHAND_NZ(pthread_create(th+i+2, NULL, task_worker, (void *)(wor_arg+i)));

  /* wait computation end */
  for (i=0; i<parallDegree+2; i++)
    if (i != 1)
      pthread_join(th[i], NULL);

  return 0;
}

void *
task_generator(void *arg)
{
  gen_arg_t *	Arg = arg;
  register void *	ch = Arg->ch_out;
  register int		k = streamSizeK;
  register int		interarrival = Arg->interarrival;
  int i, msg = 123, cpu = Arg->cpu;
  uint_reg_t start;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  printf("generator: cpu &msg ch_out %d %p %p\n", cpu, (void *)&msg, ch);

  pthread_barrier_wait(&start_computation);
  
  for (i=0; i<k; i++) {
#ifdef DEBUG
    printf("DBG: gen: start %d\n", i);
#endif
    ch_sym_ref_ad1_udn_send(ch, &msg);
    start = get_clock_cycle;
    while (get_clock_cycle - start < interarrival) ;
  }

  pthread_barrier_wait(&end_computation);

  return (void *)NULL;
}
    
void *
task_worker(void *arg)
{
  worker_arg_t *	Arg = arg;
  register void *	ch_in = Arg->ch_in;
  register void *	ch_out = Arg->ch_out;
  register void *	ch_out_left = Arg->ch_out_left;
  register void *	ch_out_right = Arg->ch_out_right;
  register void *	ch_out_result = ch_results;
  register int		k = streamSizeK;
  int i, *vtg, result[2], cpu = Arg->cpu, rank = Arg->rank;
  uint_reg_t start;


  result[1] = rank;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  printf("worker-%d: cpu ch_in ch_out ch_out_left ch_out_right %d %p %p %p %p\n",
	 rank, cpu, ch_in, ch_out, ch_out_left, ch_out_right);

  pthread_barrier_wait(&start_computation);

  for (i=0; i<k; i++) {
#ifdef DEBUG
    printf("DBG: wor-%d: start %d\n", rank, i);
#endif
    vtg = ch_sym_ref_ad1_udn_receive(ch_in);
#ifdef PIPELINE
    if (NULL != ch_out)
      /* i'm not the last in pipeline */
      ch_sym_ref_ad1_udn_send(ch_out, vtg);
#elif defined DEPTH_BINTREE
    if (NULL != ch_out_left)
      /* i'm not a leaf in tree */
      ch_sym_ref_ad1_udn_send(ch_out_left, vtg);
    if (NULL != ch_out_right)
      /* i have a right child in tree */
      ch_sym_ref_ad1_udn_send(ch_out_right, vtg);    
#endif
    if (NULL == vtg || 123 != *(int *)vtg)
      printf("worker-%d: err in line %d\n", rank, __LINE__);
    else {
      result[0] = *vtg + rank;
      start = get_clock_cycle;
      while (get_clock_cycle - start < worker_computation_latency) ;
    }
    ch_asymin_ref_ad1_udn_send_2(ch_out_result, &result);
  }

  pthread_barrier_wait(&end_computation);

  return (void *)NULL;
}
    
void *
task_collector(void *arg)
{
  col_arg_t *		Arg = arg;
  register void *	ch = Arg->ch_in;
  register int		k = streamSizeK;
  register int		n = parallDegree;
  int i, j, *vtg, cpu = Arg->cpu;

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  printf("collector: cpu ch_in %d %p\n", cpu, ch);

  pthread_barrier_wait(&start_computation);

  for (i=0; i<k; i++) { 
    for (j=0; j<n; j++) {
#ifdef DEBUG
      printf("DBG: col: start %d %d\n", i, j);
#endif
      vtg = ch_asymin_ref_ad1_udn_receive(ch);
      if (NULL == vtg) printf("collector: err in line %d\n", __LINE__);
      else if (123 + *((int *)vtg+1) != *(int *)vtg) 
	printf("collector: err: iteration %d value is %d\n", i, *(int *)vtg);
      else {
#ifdef DEBUG
	printf("DBG: col: received from %d\n", *((int *)vtg+1));
#endif
	; // do nothing
      }
    }
  }

  pthread_barrier_wait(&end_computation);

  return (void *)NULL;
}

void
create_pipeline(worker_arg_t *array, int length, gen_arg_t *generator)
{
  int i;
  for (i=0; i<length; i++) {
    if (i != length-1) {
      ERRHAND_NN(array[i].ch_out =
		 ch_sym_ref_ad1_udn_create(array[i].cpu, array[i+1].cpu, 1, 0));
    } else
      array[i].ch_out = NULL;
    if (i != 0)
      array[i].ch_in = array[i-1].ch_out;
    else
      array[0].ch_in = generator->ch_out;
  }
}

/**
 * node		array di parametri di un worker
 * length	numero di elementi nell'array node
 * height	altezza albero binario mappato sull'array node
 * index	indice corrente nell'array node
 * parent_index indice del padre nell'albero mappato in node
 * cpus		insieme di cpu dataplane disponibili per i workers, i workers
 *		sono disposti in modo sequenziale su tale insieme di cpu
 */
void
create_depth_tree(worker_arg_t *node, int length, int height, int index,
		  int parent_index, gen_arg_t *generator)
{
  /* Alla mia struttura node[index] creo ed imposto ch_in. I miei
   * ch_out_left e ch_out_right saranno impostati rispettivamente dal
   * mio figlio sinistro e dal mio figlio destro destro
   */
  int left_index, right_index;

  if (-1 == parent_index) {
    /* I'M THE ROOT */
    /* set my input channel */
    node[index].ch_in = generator->ch_out;
  } else { 
    /* set my input channel */
    if (index == parent_index + 1) {
      /* I'M THE LEFT CHILD */
      node[index].ch_in =
	ch_sym_ref_ad1_udn_create(node[parent_index].cpu,
				  node[index].cpu, 1, 0);
    } else {
      /* I'M THE RIGHT CHILD */
      node[index].ch_in = 
	ch_sym_ref_ad1_udn_create(node[parent_index].cpu,
				  node[index].cpu, 2, 0);
    }      
    /* set output channel of MY PARENT, from my parent to me */
    if (index == parent_index + 1)
      node[parent_index].ch_out_left = node[index].ch_in;
    else
      node[parent_index].ch_out_right = node[index].ch_in;
  }
  /* set null my output channels */
  node[index].ch_out_left = NULL;
  node[index].ch_out_right = NULL;
  if (height > 0) {
    /* I'M NOT A LEAF */
    left_index = index + 1;
    right_index = index + (1 << height);
    if (left_index < length)
      create_depth_tree(node, length, height-1, left_index, index, generator);
    if (right_index < length)
      create_depth_tree(node, length, height-1, right_index, index, generator);
  } 
}

/**
 * mappa i threads worker della computazione sulle cpu dataplane
 * disponibili scrivendo i valori di 'cpu' e 'rank' negli elementi
 * dell'array 'wor_arg'. L'insieme di cpu 'dataplanes' deve contenere
 * solo quelle cpu dataplane disponibili per la sola esecuzione di un
 * unico worker
 */
int
worker_cpu_mapping_sequential(worker_arg_t *wor_arg, int parallDegree,
			      const cpu_set_t *dataplanes)
{
  int i;
  for (i=0; i<parallDegree; i++) {
    if (-1 == (wor_arg[i].cpu = tmc_cpus_find_nth_cpu(dataplanes, i)))
      return -1;
    wor_arg[i].rank = i;
  }
  return 0;
}

void
test_create_depth_tree(worker_arg_t *array, int size)
{
  int i;
  for (i=0; i<size; i++) 
    printf("i cpu[i] ch_in[i] ch_out_left[i] ch_out_right[i] %d %d %p %p %p \n",
	   i, array[i].cpu, array[i].ch_in, array[i].ch_out_left,
	   array[i].ch_out_right);
} 
