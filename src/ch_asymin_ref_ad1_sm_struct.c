#include <stdlib.h>
#include <errno.h>
#include <tmc/alloc.h>
#include <tmc/cpus.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_asymin_ref_ad1_sm_struct.h"
#include "ch_asymin_ref_ad1_sm_struct_private.h"

ch_asymin_ref_ad1_sm_struct_t *
ch_asymin_ref_ad1_sm_struct_create_2(int cpu_rcv, int cpu_snd[], void *param)
{
  return ch_asymin_ref_ad1_sm_struct_create(cpu_rcv, cpu_snd);
}

ch_asymin_ref_ad1_sm_struct_t *
ch_asymin_ref_ad1_sm_struct_create(int cpu_rcv, int cpu_snd[])
{
  ch_asymin_ref_ad1_sm_struct_t *result;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;
  int i;

  if (NULL ==
      (result = tmc_alloc_map(&alloc_descr, sizeof(ch_asymin_ref_ad1_sm_struct_t))))
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
    result->out_ref[cpu_snd[i]]->mycpu = cpu_snd[i];
    result->out_ref[cpu_snd[i]]->in_ref = (void *)result->in_ref;
  }
  if (MAX_CPU == i) { errno = EINVAL; return NULL; }
  result->cpu_rcv = cpu_rcv;
  result->num_multi = i;
  result->in_ref->last_rcved = i-1;

  return result;
}

void
ch_asymin_ref_ad1_sm_struct_init_sender(ch_asymin_ref_ad1_sm_struct_t **ch)
{
  int mycpu =  tmc_cpus_get_my_cpu();
  *ch = (ch_asymin_ref_ad1_sm_struct_t *) (*ch)->out_ref[mycpu];
}

void
ch_asymin_ref_ad1_sm_struct_destroy(ch_asymin_ref_ad1_sm_struct_t **ch)
{
  int i;
  tmc_alloc_unmap((void *)(*ch)->in_ref, sizeof(in_rw_t));
  for (i=0; i<MAX_CPU; i++)
    if (NULL != (*ch)->out_ref[i])
      tmc_alloc_unmap((void *)(*ch)->out_ref[i], sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_asymin_ref_ad1_sm_struct_t));  
  *ch = NULL;
}

void
ch_asymin_ref_ad1_sm_struct_send(ch_asymin_ref_ad1_sm_struct_t *ch, void *msg)
{
  register int		mycpu = ((out_rw_t *)ch)->mycpu;
  register in_rw_t *	in_interf = ((out_rw_t *)ch)->in_ref;

  while (0 == ((volatile out_rw_t *)ch)->ack);

  in_interf->vtg[mycpu] = msg;
  tmc_mem_fence();
  in_interf->rdy[mycpu] = 1;
  ((out_rw_t *)ch)->ack = 0;

}


void *
ch_asymin_ref_ad1_sm_struct_receive(ch_asymin_ref_ad1_sm_struct_t *ch)
{
  register void *	result = NULL;
  register in_rw_t *	in_interf = (in_rw_t *)ch->in_ref;
  register int		i = in_interf->last_rcved;

  i = (i == MAX_CPU-1) ? (0) : (i+1);
  while (0 == in_interf->rdy[i])
    i = (i == MAX_CPU-1) ? (0) : (i+1);

  in_interf->last_rcved = i;
  result = in_interf->vtg[i];
  atomic_compiler_barrier();
  in_interf->rdy[i] = 0;
  ch->out_ref[i]->ack = 1;

  return result;
}
