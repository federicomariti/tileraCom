#include <stdlib.h>
#include <tmc/alloc.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_sym_ref_ad1_sm_nullack.h"
#include "ch_sym_ref_ad1_sm_nullack_private.h"

ch_sym_ref_ad1_sm_nullack_t *
ch_sym_ref_ad1_sm_nullack_create(int cpu_snd, int cpu_rcv, void *param)
{
  ch_sym_ref_ad1_sm_nullack_t *result = NULL;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;

  tmc_alloc_set_home(&alloc_descr, cpu_rcv);
  tmc_alloc_set_home(&alloc_rcv, cpu_rcv);
  tmc_alloc_set_home(&alloc_snd, cpu_snd);

  if (NULL == (result = tmc_alloc_map(&alloc_descr,
				      sizeof(ch_sym_ref_ad1_sm_nullack_t))))
    return NULL;
  if (NULL == (result->in_ref = tmc_alloc_map(&alloc_rcv, sizeof(in_rw_t))))
    return NULL;
  if (NULL == (result->out_ref = tmc_alloc_map(&alloc_snd, sizeof(out_rw_t))))
    return NULL;

  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->in_ref->vtg = NULL;
  result->out_ref->ack = 1;

  return result;
}

void
ch_sym_ref_ad1_sm_nullack_destroy(ch_sym_ref_ad1_sm_nullack_t **ch)
{
  tmc_alloc_unmap((void *)(*ch)->in_ref, sizeof(in_rw_t));
  tmc_alloc_unmap((void *)(*ch)->out_ref, sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_sym_ref_ad1_sm_nullack_t));
  *ch = NULL;
}

void
ch_sym_ref_ad1_sm_nullack_send(ch_sym_ref_ad1_sm_nullack_t *ch,
			       void *msg)
{
  while (0 == ch->out_ref->ack)
    ;
  ch->out_ref->ack = 0;
  atomic_compiler_barrier();
  ch->in_ref->vtg = msg;
}


void *
ch_sym_ref_ad1_sm_nullack_receive(ch_sym_ref_ad1_sm_nullack_t *ch)
{
  void *result = NULL;

  while (NULL == ch->in_ref->vtg)
    ;
  result = (void *)ch->in_ref->vtg;
  ch->in_ref->vtg = NULL;
  atomic_compiler_barrier();
  ch->out_ref->ack = 1;

  return result;
}
