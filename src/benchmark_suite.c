#include <sys/time.h>
/*
 * Il comportamento predefinito \`e la misurazione del tempo di
 * completamento dello stream.
 *
 * Attraverso l'uso di macro si pu\`o specificare il tipo dei dati, la
 * modalit\`a di collezionamento, oppure imporre la misurazione di un
 * diverso parametro.
 *
 * Macro:
 *
 * GATHER
 *
 * specifica se il collettore fa da gather o meno: 
 *
 * 0: il collettore effettua solo la receive, i workers scrivono il
 * risultato direttamente nel vettore risultato C. Cio` e` usabile
 * solo con un numero di workers multiplo di M
 *
 * 1: il collettore riceve il riferimento di una struttura dati dove
 * il worker ha salvato i suoi risulati, quindi copia i valori
 * contenuti in tale struttura nel vettore risultato
 *
 * DATATYPE
 *
 * specifica il tipo dei dati
 * 0: int
 * 1: float
 *
 * CALC_TIME
 *
 * se definita viene preso il tempo di calcolo di un singolo
 * worker. Per tempo di calcolo si intende l'attivita` di
 * moltiplicazione righe per colonna svolta una volta ricevuta la
 * matrice ed eseguito il multicast.
 *
 * CALC_TIME_ROW
 *
 * se definita viene preso il tempo di calcolo di un singolo elemento,
 * ovvero il tempo impiegato per una singola moltiplicazione riga per
 * colonna.
 *
 * ASYM_TIME
 *
 * se definita viene preso il tempo per effettuare l'invio sul canale
 * del collector.
 *
 * MULT_TIME
 *
 * se definita viene preso il tempo per effettuare la multicast
 * distribuita nei workers
 *
 * WORKERS_SERVICE_TIME
 *
 * se definita viene preso il tempo di servizio del sottosistema dei
 * workers
 *
 * DONT_WRITE_RESULT
 *
 * se definita non vengono scritti i risultati nella directory
 * outs_benchmark_suite
 *
 * se definita non si scrive il risultato nel file.
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h> 
#include <math.h>
#include <getopt.h>
#include <tmc/cpus.h>
#include <tmc/udn.h>
#include <tmc/task.h>
#include <tmc/alloc.h>

#include <arch/atomic.h>

#include "error_handler.h"
#include "ch_sym_ref_ad1_udn.h"
#include "ch_sym_ref_ad1_sm_fence.h"
#include "ch_sym_ref_ad1_sm_nullack.h"
#include "ch_asymin_ref_ad1_udn.h"
#include "ch_asymin_ref_ad1_sm_param_b.h"

////////////////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////////////////

#define ATOMIC_GET_CLOCK_CYCLE(VAR)		\
  atomic_compiler_barrier();			\
  (VAR) = __insn_mfspr(SPR_CYCLE_LOW);		\
  atomic_compiler_barrier();

#define DONT_MOVE(EXP)				\
  atomic_compiler_barrier();			\
  EXP;						\
  atomic_compiler_barrier();

/**
 * Massimo numero di workers allocabili:
 *
 *  Si ipotizza una configurazione con tutte le cpu dataplane eccetto
 *  le ultime due.
 *
 *  Due cpu dataplane sono necessarie per eseguire il Generatore e il
 *  Collettore.
 *
 *  Una cpu dataplane e` necessaria per eseguire il task
 *  principale. Nota: e` necessario che tale task sia eseguito in una
 *  cpu dataplane e con UDN attivo in quanto l'inizializzazione dei
 *  canali puo far uso della rete UDN.
 */
#define MAX_PARALLDEGREE 59

/*
 * Gathering:	GATHER == 1 <=> gathering
 *		GATHER == 0 <=> no gathering
 * Data Type:	DATATYPE == 0 <=> int
 *		DATATYPE == 1 <=> float
 */
#ifndef GATHER
# define GATHER 1
#endif
#ifndef DATATYPE
# define DATATYPE 0
#endif

#if GATHER == 1 && DATATYPE == 0
# define VERSION_STRING "gatherInt"
#elif GATHER == 0 && DATATYPE == 0
# define VERSION_STRING "nogatherInt"
#elif GATHER == 1 && DATATYPE == 1
# define VERSION_STRING "gatherFloat"
#elif GATHER == 0 && DATATYPE == 1
# define VERSION_STRING "nogatherFloat"
#endif

#define get_clock_cycle __insn_mfspr(SPR_CYCLE_LOW)
#define CYCLE_LENGTH_MILLI 0.0000011566265
#define CYCLE_LENGTH_MICRO 0.0011566265
#define CYCLE_LENGTH_NANO 1.1566265
#define NUM_CPU 60
#define DEFAULT_SUITE_LENGTH 10
#define DEFAULT_M 160
#define DEFAULT_K 1000
#define DEFAULT_INTERARRIVAL 1

#define IMPL_SYM_UDN 0
#define IMPL_SYM_SM_FENCE 1
#define IMPL_SYM_SM_NULLACK 2
#define IMPL_ASYMIN_UDN 0
#define IMPL_ASYMIN_SM 1

#define setOpt(VAR, NAME, HAS_ARG, FLAG, VAL) (VAR).name = NAME; (VAR).has_arg = HAS_ARG; (VAR).flag = FLAG; (VAR).val = VAL;

typedef struct {
  void *	(*create)(int, int, void *);
  void		(*send)(void *, void *);
  void *	(*receive)(void *);
  void		(*destroy)(void **);

} ch_impl_t;

typedef struct {
  void *	(*create)(int, int[], void *);
  void		(*send)(void *, void *, int);
  void *	(*receive)(void *);
  void		(*destroy)(void **);

} ch_asym_impl_t;

typedef struct {
  int		cpu;
  int		rank;
  void		*ch_in;
  void		*ch_out_left;
  void		*ch_out_right;
#if DATATYPE==0
  int		*vectorB;
#elif DATATYPE==1
  float		*vectorB;
#endif
#if GATHER==0 && DATATYPE==0
  int		*vectorC_set;
#elif GATHER==0 && DATATYPE==1
  float		*vectorC_set;
#endif
  
} worker_arg_t;

typedef struct {
  int		cpu;
  void		*ch_out;
  int		interarrival;
#if DATATYPE==0
  int		**matrixA_set;
#elif DATATYPE==1
  float		**matrixA_set;
#endif
} gen_arg_t;

typedef struct {
  int		cpu;
  void		*ch_in;
#if DATATYPE==0
  int		*vectorC_set;
#elif DATATYPE==1
  float		*vectorC_set;
#endif
} col_arg_t;

////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////

void *task_generator(void *arg);
void *task_worker(void *arg);
void *task_collector(void *arg);
int worker_cpu_mapping_sequential(worker_arg_t *wor_arg, int parallDegree,
				  const cpu_set_t *dataplanes);
void create_depth_tree(worker_arg_t *node, int length, int height, int node_index,
		       int parent_index, gen_arg_t *generator);
void test_create_depth_tree(worker_arg_t *array, int size);

int			suite_length = DEFAULT_SUITE_LENGTH;
int			dataSizeM = DEFAULT_M;
int			streamSizeK = DEFAULT_K;
int			interarrival = DEFAULT_INTERARRIVAL;
int			parallDegree;
void			*ch_results;
pthread_barrier_t	start_computation;
pthread_barrier_t	end_computation;

uint_reg_t		a, b;

int			sym_impl = 0;
int			asym_impl = 0;
int			multicast_impl = 0;

/**
 * Usato insieme a impl_sym per invocare la giusta implementazione
 * nell'invio e nella ricezione
 */
ch_impl_t channel_sym_fun[] = {
  { ch_sym_ref_ad1_udn_create_2,
    ch_sym_ref_ad1_udn_send,
    ch_sym_ref_ad1_udn_receive,
    ch_sym_ref_ad1_udn_destroy
  },
  { ch_sym_ref_ad1_sm_fence_create,
    ch_sym_ref_ad1_sm_fence_send,
    ch_sym_ref_ad1_sm_fence_receive,
    ch_sym_ref_ad1_sm_fence_destroy
  },
  { ch_sym_ref_ad1_sm_nullack_create,
    ch_sym_ref_ad1_sm_nullack_send,
    ch_sym_ref_ad1_sm_nullack_receive,
    ch_sym_ref_ad1_sm_nullack_destroy
  }
};
/**
 * Usato insieme a impl_asym per invocare la giusta implementazione
 * nell'invio e nella ricezione
 */
ch_asym_impl_t channel_asym_fun[] = {
  { ch_asymin_ref_ad1_udn_create_2,
    ch_asymin_ref_ad1_udn_send,
    ch_asymin_ref_ad1_udn_receive,
    ch_asymin_ref_ad1_udn_destroy
  },
  { ch_asymin_ref_ad1_sm_param_b_create_2,
    ch_asymin_ref_ad1_sm_param_b_send,
    ch_asymin_ref_ad1_sm_param_b_receive,
    ch_asymin_ref_ad1_sm_param_b_destroy
  }
};

FILE *complTime_file, *calctime_file, *calctimerow_file, *asymTime_file, *multTime_file, *serviceTime_file;


////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
  struct timeval time_start_comp, time_end_comp, time_comp;

  ch_sym_ref_ad1_udn_param_t udn_param;

  pthread_t	th[NUM_CPU];
  int		main_cpu;
  int		gen_cpu = 0;
  int		col_cpu = 1;
  cpu_set_t	hardwall, dp, dp_cur;
  int		h, i, j, l;
  gen_arg_t	gen_arg;
  col_arg_t	col_arg;
  worker_arg_t	wor_arg[NUM_CPU];
  int		worker_cpus[NUM_CPU];
#if DATATYPE == 0
  int		**matrixA_set, *vectorB, *vectorC_set;
  size_t	matrixA_size;
  size_t	vectorB_size;
  size_t	vectorC_set_size;
#elif DATATYPE == 1
  float		**matrixA_set, *vectorB, *vectorC_set;
  size_t	matrixA_size;
  size_t	vectorB_size;
  size_t	vectorC_set_size;
#endif
  tmc_alloc_t	alloc = TMC_ALLOC_INIT;
  char		complTime_filename[80];
#ifdef CALC_TIME
  char		calctime_filename[80];
#endif
#if defined CALC_TIME_ROW || defined CALC_TIME
  char		calctimerow_filename[80];
#endif
#ifdef ASYM_TIME
  char		asymTime_filename[80];
#endif
#ifdef MULT_TIME
  char		multTime_filename[80];
#endif
#ifdef WORKERS_SERVICE_TIME
  char		serviceTime_filename[80];
#endif

  /* parse command line arguments */
  int opt = 0;
  int longopt = 0;
  struct option options[11];
  setOpt(options[0],	"streamLength",	required_argument,	&longopt,	'K');
  setOpt(options[1],	"dataSize",	required_argument,	&longopt,	'M');
  setOpt(options[3],	"interarrival",	required_argument,	&longopt,	'i');  
  setOpt(options[4],	"sym_impl",	required_argument,	&longopt,	's');
  setOpt(options[5],	"asym_impl",	required_argument,	&longopt,	'a');  
  setOpt(options[6],	"multicast",	required_argument,	&longopt,	'm');
  setOpt(options[7],	"cpu-mapping",	required_argument,	&longopt,	'c');
  setOpt(options[8],	"suite-length",	required_argument,	&longopt,	'l');
  setOpt(options[9],	"help",		required_argument,	&longopt,	'h');
  setOpt(options[10],	NULL,		0,			NULL,		0);
  while (longopt || -1 != (opt = getopt_long(argc, argv, "l:K:M:s:a:m:c:hi:", options, NULL))) {
    switch (opt) {
    case 'l':	suite_length = atoi(optarg); break;
    case 'K':   streamSizeK = atoi(optarg); break;
    case 'M':   dataSizeM = atoi(optarg); break;
    case 'i':   interarrival = atoi(optarg); break;
    case 's':
      /* symmetric channels implemetation */
      if (!strcmp("udn", optarg)) sym_impl=IMPL_SYM_UDN;
      else if (!strcmp("sm_fence", optarg)) sym_impl=IMPL_SYM_SM_FENCE;
      else if (!strcmp("sm_nullack", optarg)) sym_impl=IMPL_SYM_SM_NULLACK;
      break;
    case 'a':
      /* asymmetric channels implementations */
      if (!strcmp("udn", optarg)) asym_impl=IMPL_ASYMIN_UDN;
      else if (!strcmp("sm", optarg)) asym_impl=IMPL_ASYMIN_SM;
      break;
    case 'm':
      /* multicast implementation */
      if (!strcmp("pipe", optarg)) multicast_impl = 0;
      else if (!strcmp("tree", optarg)) multicast_impl = 1;
      break;
    case 'c':
      /* cpu mapping */
      break;
    case 'h':
      printf("Usage: benchmark [OPTIONS]\nOptions:\n"
	     "  -K NUM, --streamLength=NUM \n"
	     "  -M NUM, --dataSize=NUM \n"
	     "  -n NUM, --parallDegree=NUM \n"
	     "  -i NUM, --interarrival=NUM \n"
	     "  -s {udn,sm_fence,sm_nullack}, --sym_impl={udn,sm_fence,sm_nullack} \n"
	     "  -a {udn,sm}, --asym_impl={udn,sm} \n"
	     "  -m {pipe,tree}, --multicast={pipe,tree} \n"
	     "  -c {linear,adhoc:NUM,NUM,...}, --cpu-mapping={linear,adhoc:NUM,NUM,...} \n"
	     "  -h, --help"
	     "\n");
      break;
    case 0:    opt=longopt; continue;
    }
    longopt = 0;
  }

  ERRHAND(gettimeofday(&time_start_comp, NULL));

  printf("streamSize dataSize interarrivalServiceT symImpl asymImpl"
	 "   %d %d %d %d %d\n",
	 streamSizeK, dataSizeM, interarrival, sym_impl, asym_impl);

#ifdef CALC_TIME
  sprintf(calctime_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_calctime.dbg",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (calctime_file = fopen(calctime_filename, "a")))
    perror("create calctime file");

  sprintf(calctimerow_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_calctimerow.dbg",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (calctimerow_file = fopen(calctimerow_filename, "a")))
    perror("create calctimerow file");
#endif

#ifdef CALC_TIME_ROW
  sprintf(calctimerow_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_calctimerow_2.dbg",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (calctimerow_file = fopen(calctimerow_filename, "a")))
    perror("create calctimerow file");
#endif

#ifdef ASYM_TIME
  sprintf(asymTime_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_asymtime.dbg",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (asymTime_file = fopen(asymTime_filename, "a")))
    perror("create asymtime file");
#endif

#ifdef MULT_TIME
  sprintf(multTime_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_multictime.dbg",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (multTime_file = fopen(multTime_filename, "a")))
    perror("create multicastTime file");
#endif

#ifdef WORKERS_SERVICE_TIME
  sprintf(serviceTime_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d_servicetime.dat",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (serviceTime_file = fopen(serviceTime_filename, "a")))
    perror("create serviceTime file");
#endif

#if !defined WORKERS_SERVICE_TIME && ! defined MULT_TIME && ! defined CALC_TIME && ! defined CALC_TIME_ROW && ! defined ASYM_TIME && ! defined DONT_WRITE_RESULT
  /* write Completion Time (Tc) in file if and only if are not taken
   * other measures */
  sprintf(complTime_filename,
	  "outs_benchmark_suite/"VERSION_STRING"_%d%d_%d_%d_%d.dat",
	  sym_impl, asym_impl, streamSizeK, interarrival, dataSizeM);
  if (NULL == (complTime_file = fopen(complTime_filename, "a")))
    ;
#endif

#if DATATYPE == 0
  matrixA_size = sizeof(int)*dataSizeM*dataSizeM;
  vectorB_size = sizeof(int)*dataSizeM;
  vectorC_set_size = sizeof(int)*streamSizeK*dataSizeM;  
#elif DATATYPE == 1
  matrixA_size = sizeof(float)*dataSizeM*dataSizeM;
  vectorB_size = sizeof(float)*dataSizeM;
  vectorC_set_size = sizeof(float)*streamSizeK*dataSizeM;
#endif

  /* setup udn hardwall */
  ERRHAND(tmc_cpus_get_dataplane_cpus(&dp));
  memcpy(&hardwall, &dp, sizeof(cpu_set_t));
  /* set my cpu (dataplane) */
  ERRHAND(main_cpu = tmc_cpus_find_last_cpu(&dp));
  ERRHAND(tmc_cpus_set_my_cpu(main_cpu));
  tmc_cpus_remove_cpu(&dp, main_cpu);

  /* do suite_length iteatrations of benchmark */
  for (l=0; l<suite_length; l++) {
    printf("suite number %d\n", l);
    /* execute benchmark for all parallelism degree */
    for (parallDegree=1; parallDegree<MAX_PARALLDEGREE; parallDegree++) {
      if (0 != (dataSizeM % parallDegree)) continue;

      printf("suite %d, parallel degree = %d\n", l, parallDegree);

      memcpy(&dp_cur, &dp, sizeof(cpu_set_t));
    
      memset(&gen_arg, 0, sizeof(gen_arg));
      memset(&col_arg, 0, sizeof(col_arg));
      memset(&wor_arg, 0, sizeof(wor_arg));

      /* initialize udn hardwall with all dataplane cpus */
      ERRHAND(tmc_udn_init(&hardwall));

      /* allocate and initialize data structures */
      ERRHAND_NN(matrixA_set = tmc_alloc_map(&alloc, streamSizeK * sizeof(void *)));
      for (h=0; h<streamSizeK; h++) {
	ERRHAND_NN(matrixA_set[h] = tmc_alloc_map(&alloc, matrixA_size));
	for (i=0; i<dataSizeM; i++)
	  for (j=0; j<dataSizeM; j++)
	    *(matrixA_set[h]+i*dataSizeM+j) = dataSizeM*i + j;
      }
      ERRHAND_NN(vectorB = tmc_alloc_map(&alloc, vectorB_size));
      for (i=0; i<dataSizeM; i++) vectorB[i] = i;
      ERRHAND_NN(vectorC_set = tmc_alloc_map(&alloc, vectorC_set_size));

      /* setting task args: cpus and worker rank */
      ERRHAND(gen_arg.cpu = tmc_cpus_find_nth_cpu(&dp_cur, gen_cpu));
      ERRHAND(col_arg.cpu = tmc_cpus_find_nth_cpu(&dp_cur, col_cpu));
      tmc_cpus_remove_cpu(&dp_cur, gen_arg.cpu);
      tmc_cpus_remove_cpu(&dp_cur, col_arg.cpu);
      worker_cpu_mapping_sequential(wor_arg, parallDegree, &dp_cur); /* <-- ranks */
  
      /* setting task args: channels */
      udn_param.dq_snd = 1;
      udn_param.dq_rcv = 0;
      gen_arg.ch_out =
      	channel_sym_fun[sym_impl].create(gen_arg.cpu, wor_arg[0].cpu, &udn_param);
      
      create_depth_tree(wor_arg, parallDegree, log2(parallDegree), 0, -1, &gen_arg);
      //[DEBUG] test_create_depth_tree(wor_arg, parallDegree);
      for (i=0; i<parallDegree; i++) {
	/* array necessario per il costruttore di un canale asimmetrico */
	worker_cpus[i] = wor_arg[i].cpu;
      }
      worker_cpus[parallDegree] = -1;

      udn_param.dq_snd = 3;
      udn_param.dq_rcv = 0;
      ch_results =
	channel_asym_fun[asym_impl].create(col_arg.cpu, worker_cpus, &udn_param);

      col_arg.ch_in = ch_results;

      /* setting task args: others */
      gen_arg.interarrival = interarrival;
      gen_arg.matrixA_set = matrixA_set;
      for (i=0; i<parallDegree; i++) {
	wor_arg[i].vectorB = vectorB;
#if GATHER == 0
	wor_arg[i].vectorC_set = vectorC_set;
#endif
      }
      col_arg.vectorC_set = vectorC_set;
  
  
      /* start computation */
      ERRHAND_NZ(pthread_barrier_init(&start_computation, NULL, parallDegree+2));
      ERRHAND_NZ(pthread_barrier_init(&end_computation, NULL, parallDegree+2));
      ERRHAND_NZ(pthread_create(th, NULL, task_generator, (void *)&gen_arg));
      ERRHAND_NZ(pthread_create(th+1, NULL, task_collector, (void *)&col_arg));
      for (i=0; i<parallDegree; i++)
	ERRHAND_NZ(pthread_create(th+i+2, NULL, task_worker, (void *)(wor_arg+i)));

      /* wait computation end */
      for (i=0; i<parallDegree+2; i++)
	pthread_join(th[i], NULL);

      /* destory environment */
      for (i=0; i<streamSizeK; i++)
	ERRHAND(tmc_alloc_unmap(matrixA_set[i], matrixA_size));
      ERRHAND(tmc_alloc_unmap(matrixA_set, streamSizeK * sizeof(void *)));
      ERRHAND(tmc_alloc_unmap(vectorB, vectorB_size));
      ERRHAND(tmc_alloc_unmap(vectorC_set, vectorC_set_size));
      channel_sym_fun[sym_impl].destroy(&gen_arg.ch_out);
      channel_asym_fun[asym_impl].destroy(&col_arg.ch_in);
      for (i=0; i<parallDegree; i++) {
	if (NULL != wor_arg[i].ch_out_left)
	  channel_sym_fun[sym_impl].destroy(&wor_arg[i].ch_out_left);
	if (NULL != wor_arg[i].ch_out_right)
	  channel_sym_fun[sym_impl].destroy(&wor_arg[i].ch_out_right);
      }
      ERRHAND(tmc_udn_close());

    } /* end for (parallDegree=1; parallDegree<MAX_PARALLDEGREE; parallDegree++) */
  } /* end for (l=0; l<suite_length; l++) */
  
  ERRHAND(gettimeofday(&time_end_comp, NULL));
  timersub(&time_end_comp, &time_start_comp, &time_comp);
  printf("\ntime elapsed: %ld sec %f msec\n", time_comp.tv_sec, time_comp.tv_usec/(double)1000);

  if (NULL != complTime_file) fclose(complTime_file);
  if (NULL != calctime_file) fclose(calctime_file);
  if (NULL != calctimerow_file) fclose(calctimerow_file);

  return 0;
}

void *
task_generator(void *arg)
{
  gen_arg_t 		*Arg = arg;
  register void 	*ch = Arg->ch_out;
#if DATATYPE == 0
  register int		**A = Arg->matrixA_set;
#elif DATATYPE == 1
  register float 	**A = Arg->matrixA_set;
#endif
  register int		interarrival = Arg->interarrival;
  register int		K = streamSizeK;
  register void 	(*send)(void *, void *) = channel_sym_fun[sym_impl].send;
  int			i, cpu = Arg->cpu;
  uint_reg_t		start;

#ifdef DEBUG
  printf("[DBG] generator: cpu ch_out mtrA %d %p %p\n", cpu, ch, A);
#endif

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  pthread_barrier_wait(&start_computation);
  ATOMIC_GET_CLOCK_CYCLE(a);
  
  for (i=0; i<K; i++) {
    ATOMIC_GET_CLOCK_CYCLE(start);
    send(ch, (void *)(A[i]));
    while (get_clock_cycle - start < interarrival) ;
  }

  pthread_barrier_wait(&end_computation);

  return (void *)NULL;
}
    
void *
task_worker(void *arg)
{
  worker_arg_t *	Arg = arg;
  int			cpu = Arg->cpu;
  int			rank = Arg->rank;
  register void *	ch_in = Arg->ch_in;
  register void *	ch_out_left = Arg->ch_out_left;
  register void *	ch_out_right = Arg->ch_out_right;
  register void *	ch_out_result = ch_results;
  register int		M = dataSizeM;
  /** partition size **/
  register int		g = dataSizeM / parallDegree;
  register int		K = streamSizeK;
#if DATATYPE == 0
  register int *	B = Arg->vectorB;
  int			*A;
  /** */
  int			*result_base;
  /** */
  int			*result;
  size_t		result_size = K * (g+2) * sizeof(int);
#elif DATATYPE == 1
  register float *	B = Arg->vectorB;
  float			*A;
  float			*result;
  float			*result_base;
  size_t		result_size = K * (g+2) * sizeof(float);
#endif
  register void 	(*send)(void *, void *) = channel_sym_fun[sym_impl].send;
  register void *	(*receive)(void *) = channel_sym_fun[sym_impl].receive;
  register void		(*asym_send)(void *, void *, int) = channel_asym_fun[asym_impl].send;
  int			h, ii, i, j;
#if GATHER == 1
  tmc_alloc_t		alloc = TMC_ALLOC_INIT;
#elif DATATYPE == 0
  register int *	C = Arg->vectorC_set;
#elif DATATYPE == 1
  register float *	C = Arg->vectorC_set;
#endif

#ifdef CALC_TIME
  uint_reg_t calc_start;
  uint64_t calc_sum = 0;
#endif
#if defined CALC_TIME_ROW || defined CALC_TIME
  uint_reg_t calc_row_start;
  uint64_t calc_row_sum = 0;
#endif
#ifdef ASYM_TIME
  uint_reg_t start_asym_send;
  uint64_t asym_time = 0;
#endif
#ifdef MULT_TIME
  uint_reg_t mult_start, mult_stop;
  uint64_t mult_sum = 0;
#endif
#ifdef WORKERS_SERVICE_TIME
  uint_reg_t servicetime_start, servicetime_stop=0;
  uint64_t servicetime_sum = 0;
#endif

#ifdef DEBUG
  printf("[DBG] worker: cpu rank ch_in ch_out_l ch_out_r vecB %d %d %p %p %p %p\n",
	 cpu, rank, ch_in, ch_out_left, ch_out_right, B);
#endif

#if GATHER == 1 
  /* alloc set of my partial results (vector of g+2 integers) */
  ERRHAND_TILERA_NN(result_base = tmc_alloc_map(&alloc, result_size));
  result = result_base;
  /* for each vector: last two integer are my rank and the position in
   * the stream */
  result[g] = rank;
#else
  result_base = C;
  result = result_base + rank*g;
#endif

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  pthread_barrier_wait(&start_computation);

  for (h=0; h<K; h++,
#if GATHER == 1
	 result+=(g+2)) {
#else
	 result+=M) {
#endif

    A = receive(ch_in);

#ifdef WORKERS_SERVICE_TIME
    atomic_compiler_barrier();
    if (0 == rank) {
      servicetime_start = servicetime_stop;
      servicetime_stop = get_clock_cycle;
      if (servicetime_start != 0)
	servicetime_sum += servicetime_stop - servicetime_start;
    }
    atomic_compiler_barrier();
#endif

#ifdef MULT_TIME
    atomic_compiler_barrier();
    if (0 == rank)
      mult_start = get_clock_cycle;
    atomic_compiler_barrier();
#endif 

    if (NULL != ch_out_left)
      send(ch_out_left, A);
    if (NULL != ch_out_right)
      send(ch_out_right, A);    

#ifdef MULT_TIME
    atomic_compiler_barrier();
    if (0 == rank) {
      mult_stop = get_clock_cycle;
      mult_sum += mult_stop - mult_start;
    }
    atomic_compiler_barrier();
#endif 

#ifdef CALC_TIME
    ATOMIC_GET_CLOCK_CYCLE(calc_start); /* START CALC */
#endif

    for (ii=0, i=g*rank; ii<g; ii++, i++) {
#ifdef CALC_TIME_ROW
      ATOMIC_GET_CLOCK_CYCLE(calc_row_start);
#endif
      result[ii] = 0;
      for (j=0; j<M; j++) {
	result[ii] += *(A+i*M+j) * B[j];
      }
#ifdef CALC_TIME_ROW
      DONT_MOVE(calc_row_sum += get_clock_cycle - calc_row_start);
#endif
    }

#if defined CALC_TIME
    DONT_MOVE(calc_sum += get_clock_cycle - calc_start); /* END CALC */
#endif

#if GATHER == 1
    result[g] = rank; /* my rank */
    result[g+1] = h; /* position of stream element */
#endif

#if defined ASYM_TIME
    ATOMIC_GET_CLOCK_CYCLE(start_asym_send);
#endif

    asym_send(ch_out_result, result, rank);

#ifdef ASYM_TIME
    DONT_MOVE(asym_time += get_clock_cycle - start_asym_send);
#endif

  } /* end for (h=0; h<K; h++) */

  pthread_barrier_wait(&end_computation);



#ifdef CALC_TIME
  //print out calculation time and calculation time for a single row
  if (NULL == calctime_file) calctime_file = stdout;
  if (NULL == calctimerow_file) calctimerow_file = stdout;
  fprintf(calctime_file, "%d %f\n",
	  parallDegree, ((double)calc_sum)/K);
  fprintf(calctimerow_file, "%d %f\n",
	  parallDegree, ((double)calc_sum)/K/g);
#endif

#ifdef CALC_TIME_ROW
  //print out calculation time for a single row
  if (NULL == calctimerow_file) calctimerow_file = stdout;
  fprintf(calctimerow_file, "%d %f\n",
	  parallDegree, ((double)calc_row_sum)/(K*g));

  printf("calc_row_sum = %lu\n", (long unsigned)(calc_row_sum));
#endif

#ifdef ASYM_TIME
  //print out time for asymmetric communication
  if (NULL == asymTime_file)  asymTime_file = stdout;
  fprintf(asymTime_file, "%d %f\n",
	  parallDegree, ((double)asym_time)/K);
#endif

#ifdef MULT_TIME
  if (0 == rank) {
    if (NULL == multTime_file)  multTime_file = stdout;
    fprintf(multTime_file, "%d %f\n",
	    parallDegree, ((double)mult_sum)/K);
  }
#endif

#ifdef WORKERS_SERVICE_TIME
  if (0 == rank) {
    //#ifdef DEBUG
    printf("[DBG] worker: cpu rank ch_in ch_out_l ch_out_r vecB %d %d %p %p %p %p\n",
	   cpu, rank, ch_in, ch_out_left, ch_out_right, B);
    //#endif
    if (NULL == serviceTime_file)  serviceTime_file = stdout;
    fprintf(serviceTime_file, "%d %f\n",
	    parallDegree, ((double)servicetime_sum)/K);
  }
#endif

  ERRHAND_TILERA(tmc_alloc_unmap(result_base, result_size));

  return (void *)NULL;
}

void *
task_collector(void *arg)
{
  col_arg_t *		Arg = arg;
  register void *	ch = Arg->ch_in;
#if GATHER == 1
# if DATATYPE == 0
  register int *	C = Arg->vectorC_set;
# elif DATATYPE == 1
  register float *	C = Arg->vectorC_set;
# endif
  register const int	g = dataSizeM / parallDegree;
  register const int	M = dataSizeM;
  int			j, jj;
#endif  
  register const int	K = streamSizeK;
  register int		n = parallDegree;
  register void *	(*receive)(void *) = channel_asym_fun[asym_impl].receive;
  int			h, i, cpu = Arg->cpu;

#ifdef DEBUG
#if GATHER == 1
  printf("[DBG] collector: cpu ch_in vecC %d %p %p\n", cpu, ch, C);
#else
  printf("[DBG] collector: cpu ch_in %d %p\n", cpu, ch);
#endif
#endif

  ERRHAND_TILERA(tmc_cpus_set_my_cpu(cpu));
  ERRHAND_TILERA(tmc_udn_activate());

  pthread_barrier_wait(&start_computation);

  for (h=0; h<K; h++)
    for (i=0; i<n; i++) {
#if GATHER == 1
# if DATATYPE == 0
      register int *result = receive(ch);
# elif DATATYPE == 1
      register float *result = receive(ch);
# endif
      register int sender = result[g];
      register int stream_pos = result[g+1];

      for (jj=0, j=g*sender; jj<g; j++, jj++)
	*(C+stream_pos*M+j) = result[jj];
#else
      (void)receive(ch);
#endif
    }

  ATOMIC_GET_CLOCK_CYCLE(b);

  pthread_barrier_wait(&end_computation);

#ifdef DEBUG
  printf("[DBG] end collector\n");
#endif

  if (NULL == complTime_file) complTime_file = stdout;
  fprintf(complTime_file, "%d %f\n", n, CYCLE_LENGTH_MILLI*(b-a));

  return (void *)NULL;
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
  ch_sym_ref_ad1_udn_param_t udn_param;

  if (-1 == parent_index) {
    /* I'M THE ROOT */
    /* set my input channel */
    node[index].ch_in = generator->ch_out;
  } else { 
    /* set my input channel */
    if (index == parent_index + 1) {
      /* I'M THE LEFT CHILD */
      udn_param.dq_snd = 1;
      udn_param.dq_rcv = 0;
      node[index].ch_in =
	channel_sym_fun[sym_impl].create(node[parent_index].cpu,
					 node[index].cpu, &udn_param);
    } else {
      /* I'M THE RIGHT CHILD */
      udn_param.dq_snd = 2;
      udn_param.dq_rcv = 0;
      node[index].ch_in =
	channel_sym_fun[sym_impl].create(node[parent_index].cpu,
					 node[index].cpu, &udn_param);
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

void
test_create_depth_tree(worker_arg_t *array, int size)
{
  int i;
  for (i=0; i<size; i++) 
    printf("i cpu[i] ch_in[i] ch_out_left[i] ch_out_right[i] %d %d %p %p %p \n",
	   i, array[i].cpu, array[i].ch_in, array[i].ch_out_left,
	   array[i].ch_out_right);
} 

/**
 * mappa i threads worker della computazione sulle cpu dataplane
 * disponibili scrivendo i valori di 'cpu' e 'rank' negli elementi
 * dell'array 'wor_arg'. L'insieme di cpu 'dataplanes' deve contenere
 * quelle cpu dataplane disponibili per la sola esecuzione di un unico
 * worker
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
