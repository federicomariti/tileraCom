#include <stdlib.h>
#include <errno.h>
#include <tmc/alloc.h>
#include <tmc/cpus.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_asymin_ref_ad1_sm_param_b.h"
#include "ch_asymin_ref_ad1_sm_param_b_private.h"


ch_asymin_ref_ad1_sm_param_b_t *
ch_asymin_ref_ad1_sm_param_b_create_2(int cpu_rcv, int cpu_snd[], void *param) {
  return ch_asymin_ref_ad1_sm_param_b_create(cpu_rcv, cpu_snd);
}

ch_asymin_ref_ad1_sm_param_b_t *
ch_asymin_ref_ad1_sm_param_b_create(int cpu_rcv, int cpu_snd[])
{
  ch_asymin_ref_ad1_sm_param_b_t *result;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;
  int i;

  if (NULL ==
      (result = tmc_alloc_map(&alloc_descr, sizeof(ch_asymin_ref_ad1_sm_param_b_t))))
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
    if (NULL ==	(result->out_ref[i] = tmc_alloc_map(&alloc_snd,
						    sizeof(out_rw_t))))
      return NULL;
    result->out_ref[i]->ack = 1;
  }
  if (MAX_CPU == i) { errno = EINVAL; return NULL; }
  result->cpu_rcv = cpu_rcv;
  result->num_multi = i;
  result->in_ref->last_rcved = i-1;

  return result;
}

void
ch_asymin_ref_ad1_sm_param_b_destroy(ch_asymin_ref_ad1_sm_param_b_t **ch)
{
  int i;
  tmc_alloc_unmap((void *)(*ch)->in_ref, sizeof(in_rw_t));
  for (i=0; i<(*ch)->num_multi; i++)
    if (NULL != (*ch)->out_ref[i])
      tmc_alloc_unmap((void *)(*ch)->out_ref[i], sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_asymin_ref_ad1_sm_param_b_t));  
  *ch = NULL;
}

void
ch_asymin_ref_ad1_sm_param_b_send(ch_asymin_ref_ad1_sm_param_b_t *ch, void *msg, int rank)
{
  while (0 == ch->out_ref[rank]->ack);
  
  atomic_compiler_barrier();
  ch->in_ref->vtg[rank] = msg;
  atomic_compiler_barrier();
  tmc_mem_fence();
  atomic_compiler_barrier();
  ch->out_ref[rank]->ack = 0;
  atomic_compiler_barrier();
  ch->in_ref->rdy[rank] = 1;

}

void *
ch_asymin_ref_ad1_sm_param_b_receive(ch_asymin_ref_ad1_sm_param_b_t *ch)
{
  register void *	result = NULL;
  register in_rw_t *	in_interf = (in_rw_t *)ch->in_ref;
  register int		last_rcved = in_interf->last_rcved;
  register int		i = last_rcved;
  register int		num_multi = ch->num_multi;

  i = (i == num_multi-1) ? (0) : (i+1);
  while (0 == in_interf->rdy[i])
      i = (i == num_multi-1) ? (0) : (i+1);

  atomic_compiler_barrier();
  in_interf->last_rcved = i;
  atomic_compiler_barrier();
  result = in_interf->vtg[i];
  atomic_compiler_barrier();
  in_interf->rdy[i] = 0;
  atomic_compiler_barrier();
  ch->out_ref[i]->ack = 1;

  return result;
}

void *
ch_asymin_ref_ad1_sm_param_b_receive_(ch_asymin_ref_ad1_sm_param_b_t *ch)
{
  register void *	result = NULL;
  register int		rdy = 0;
  register int		last_rcved = ch->in_ref->last_rcved;
  register int		i = last_rcved;
  register int		num_multi = ch->num_multi;

  while (0 == rdy) {
    i = (i == num_multi-1) ? (0) : (i+1);
    while (0 == (rdy = ch->in_ref->rdy[i]) && i != last_rcved) 
      i = (i == num_multi-1) ? (0) : (i+1);
  }  

  atomic_compiler_barrier();
  ch->in_ref->last_rcved = i;
  result = ch->in_ref->vtg[i];
  ch->in_ref->rdy[i] = 0;
  atomic_compiler_barrier();
  ch->out_ref[i]->ack = 1;

  return result;
}
