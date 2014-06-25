#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <tmc/alloc.h>
#include <tmc/cpus.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_asymin_ref_ad1_sm_pthr.h"


/**
 * Dimensione di una pagina della gerarchia di memoria L2 cache - main
 * memory
 */
#define PAGE_SIZE 65536
/**
 * Dimensione in byte di una parola (intero o riferimento)
 */
#define WORD_SIZE 4

#define MAX_CPU 64


typedef struct {
  volatile int		rdy[MAX_CPU];
  void *		vtg[MAX_CPU];
  int			last_rcved;
  char			padding[PAGE_SIZE - (2*MAX_CPU+1)*WORD_SIZE];

} in_rw_t;

typedef struct {
  volatile int		ack;
  int			mycpu;
  in_rw_t *		in_ref;
  char			padding[PAGE_SIZE - 3*WORD_SIZE];

} out_rw_t;

struct ch_asymin_ref_ad1_sm_pthr_t
{
  int			cpu_snd[MAX_CPU];
  volatile out_rw_t *	out_ref[MAX_CPU];
  int			cpu_rcv;
  volatile in_rw_t *	in_ref;
  int			num_multi;
  pthread_key_t 	sender_specific_cpu;
  char			padding[PAGE_SIZE - 2*MAX_CPU*WORD_SIZE - 4*WORD_SIZE];
};


void
destroy_sender_specific_cpu(void *arg)
{
  tmc_alloc_unmap(arg, sizeof(int));
}

ch_asymin_ref_ad1_sm_pthr_t *
ch_asymin_ref_ad1_sm_pthr_create(int cpu_rcv, int cpu_snd[])
{
  ch_asymin_ref_ad1_sm_pthr_t *result;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;
  int i;

  if (NULL ==
      (result = tmc_alloc_map(&alloc_descr, sizeof(ch_asymin_ref_ad1_sm_pthr_t))))
    return NULL;

  tmc_alloc_set_home(&alloc_rcv, cpu_rcv);
  if (NULL ==
      (result->in_ref = tmc_alloc_map(&alloc_rcv, sizeof(in_rw_t))))
    return NULL;

  for (i=0; i<MAX_CPU; i++) {
    result->cpu_snd[i] = -1;
    result->out_ref[i] = NULL;
    result->in_ref->rdy[i] = 0;
  }
  for (i=0; i < MAX_CPU && -1 != cpu_snd[i]; i++) {
    result->cpu_snd[i] = cpu_snd[i];
    tmc_alloc_set_home(&alloc_snd, cpu_snd[i]);
    if (NULL ==
	(result->out_ref[cpu_snd[i]] = tmc_alloc_map(&alloc_snd,
						     sizeof(out_rw_t))))
      return NULL;
    result->out_ref[cpu_snd[i]]->ack = 1;
    result->out_ref[cpu_snd[i]]->in_ref = (void *)result->in_ref;
  }
  if (MAX_CPU == i) { errno = EINVAL; return NULL; }
  result->cpu_rcv = cpu_rcv;
  result->num_multi = i;
  result->in_ref->last_rcved = i-1;
  pthread_key_create(&result->sender_specific_cpu, destroy_sender_specific_cpu);

  return result;
}

void
ch_asymin_ref_ad1_sm_pthr_destroy(ch_asymin_ref_ad1_sm_pthr_t **ch)
{
  int i;
  tmc_alloc_unmap((void *)(*ch)->in_ref, sizeof(in_rw_t));
  for (i=0; i<MAX_CPU; i++)
    if (NULL != (*ch)->out_ref[i])
      tmc_alloc_unmap((void *)(*ch)->out_ref[i], sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_asymin_ref_ad1_sm_pthr_t));
  *ch = NULL;
}

void
ch_asymin_ref_ad1_sm_pthr_send(ch_asymin_ref_ad1_sm_pthr_t *ch, void *msg)
{
  register int *	mycpu_ref;
  register int		mycpu;
  register in_rw_t *	in_interf = (in_rw_t *)ch->in_ref;
  register out_rw_t *	out_interf;

  if (NULL != (mycpu_ref = pthread_getspecific(ch->sender_specific_cpu))) {
    mycpu = *mycpu_ref;
  }else {
    tmc_alloc_t alloc = TMC_ALLOC_INIT;
    mycpu = tmc_cpus_get_my_cpu();
    tmc_alloc_set_home(&alloc, mycpu);
    mycpu_ref = tmc_alloc_map(&alloc, sizeof(int));
    *mycpu_ref = mycpu;
    pthread_setspecific(ch->sender_specific_cpu, mycpu_ref);
  }

  out_interf = (out_rw_t *)ch->out_ref[mycpu];

  while (0 == out_interf->ack)
    ;

  in_interf->vtg[mycpu] = msg;
  tmc_mem_fence();
  in_interf->rdy[mycpu] = 1;
  out_interf->ack = 0;
}


void *
ch_asymin_ref_ad1_sm_pthr_receive(ch_asymin_ref_ad1_sm_pthr_t *ch)
{
  register void *	result = NULL;
  register int		rdy = 0;
  register int		last_rcved = ch->in_ref->last_rcved;
  register int		i = last_rcved;

  while (0 == rdy) {
    i = (i == MAX_CPU-1) ? (0) : (i+1);
    while (0 == (rdy = ch->in_ref->rdy[i]) && i != last_rcved) {
      i = (i == MAX_CPU-1) ? (0) : (i+1);
    }
  }  

  ch->in_ref->last_rcved = i;
  result = ch->in_ref->vtg[i];
  atomic_compiler_barrier();
  ch->in_ref->rdy[i] = 0;
  ch->out_ref[i]->ack = 1;

  return result;
}
