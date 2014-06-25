#include <stdlib.h>
#include <tmc/alloc.h>
#include <arch/atomic.h>

#include "ch_sym_ref_ad1_sm_null.h"
#include "ch_sym_ref_ad1_sm_null_private.h"

ch_sym_ref_ad1_sm_null_t *
ch_sym_ref_ad1_sm_null_create(int cpu_snd, int cpu_rcv, void *param)
{
  ch_sym_ref_ad1_sm_null_t *result = NULL;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;  

  tmc_alloc_set_home(&alloc_descr, cpu_rcv);
  tmc_alloc_set_home(&alloc_rcv, cpu_rcv);

  if (NULL == (result = tmc_alloc_map(&alloc_descr,
				      sizeof(ch_sym_ref_ad1_sm_null_t))))
    return NULL;
  if (NULL == (result->in_rw = tmc_alloc_map(&alloc_rcv, sizeof(in_rw_t))))
    return NULL;

  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->in_rw->vtg = NULL;

  return result;
}

void
ch_sym_ref_ad1_sm_null_destroy(ch_sym_ref_ad1_sm_null_t **ch)
{
  tmc_alloc_unmap((void *)(*ch)->in_rw, sizeof(in_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_sym_ref_ad1_sm_null_t));
  *ch = NULL;
}

void
ch_sym_ref_ad1_sm_null_send(ch_sym_ref_ad1_sm_null_t *ch,
			    void *msg)
{
  while (NULL != ch->in_rw->vtg)
    ;
  ch->in_rw->vtg = msg;
}


void *
ch_sym_ref_ad1_sm_null_receive(ch_sym_ref_ad1_sm_null_t *ch)
{
  void *result = NULL;

  while (NULL == ch->in_rw->vtg)
    ;
  result = (void *)ch->in_rw->vtg;
  ch->in_rw->vtg = NULL;

  return result;
}
