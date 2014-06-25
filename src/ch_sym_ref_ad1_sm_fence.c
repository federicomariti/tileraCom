#include <stdlib.h>
#include <tmc/alloc.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_sym_ref_ad1_sm_fence.h"
#include "ch_sym_ref_ad1_sm_fence_private.h"

#define ch_rdy in_ref->rdy
#define ch_ack out_ref->ack
#define ch_vtg in_ref->vtg

ch_sym_ref_ad1_sm_fence_t *
ch_sym_ref_ad1_sm_fence_create(int cpu_snd, int cpu_rcv, void *param)
{
  ch_sym_ref_ad1_sm_fence_t *result = NULL;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;

  tmc_alloc_set_home(&alloc_snd, cpu_snd);
  tmc_alloc_set_home(&alloc_rcv, cpu_rcv);

  if (NULL == (result = tmc_alloc_map(&alloc_descr,
				      sizeof(ch_sym_ref_ad1_sm_fence_t))))
    return NULL;
  if (NULL == (result->in_ref = tmc_alloc_map(&alloc_rcv, sizeof(in_rw_t))))
    return NULL;
  if (NULL == (result->out_ref = tmc_alloc_map(&alloc_snd, sizeof(out_rw_t))))
    return NULL;

  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->ch_rdy = 0;
  result->ch_ack = 1;

  return result;
}

void
ch_sym_ref_ad1_sm_fence_destroy(ch_sym_ref_ad1_sm_fence_t **ch)
{
  tmc_alloc_unmap((*ch)->in_ref, sizeof(in_rw_t));
  tmc_alloc_unmap((*ch)->out_ref, sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_sym_ref_ad1_sm_fence_t));
  *ch = NULL;
}

void
ch_sym_ref_ad1_sm_fence_send(ch_sym_ref_ad1_sm_fence_t *ch,
			     void *msg)
{
  while (0 == ch->ch_ack)
    ;
  atomic_compiler_barrier();
  ch->ch_vtg = msg;
  atomic_compiler_barrier();
  ch->ch_ack = 0;
  tmc_mem_fence();
  atomic_compiler_barrier();
  ch->ch_rdy = 1;
}

void *
ch_sym_ref_ad1_sm_fence_receive(ch_sym_ref_ad1_sm_fence_t *ch)
{
  void *result = NULL;

  while (0 == ch->ch_rdy)
    ;
  atomic_compiler_barrier();
  result = ch->ch_vtg;
  atomic_compiler_barrier();
  ch->ch_rdy = 0;
  atomic_compiler_barrier();
  ch->ch_ack = 1;

  return result;
}
