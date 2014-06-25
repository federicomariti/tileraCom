#include <stdlib.h>
#include <tmc/mem.h>
#include <arch/atomic.h>

#include "ch_sym_ref_ad1_sm_no.h"
#include "ch_sym_ref_ad1_sm_no_private.h"

ch_sym_ref_ad1_sm_no_t *
ch_sym_ref_ad1_sm_no_create(int cpu_snd, int cpu_rcv, void *param) {
  ch_sym_ref_ad1_sm_no_t *result = NULL;

  if (NULL == (result = malloc(sizeof(ch_sym_ref_ad1_sm_no_t))))
    return NULL;

  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->rdy = 0;
  result->ack = 1;      

  return result;
}

void
ch_sym_ref_ad1_sm_no_destroy(ch_sym_ref_ad1_sm_no_t **ch) {
  free(*ch);
  *ch = NULL;
}

void
ch_sym_ref_ad1_sm_no_send(ch_sym_ref_ad1_sm_no_t *ch,
			  void *msg) {
  while (0 == ch->ack)
    ;
  atomic_compiler_barrier();
  ch->vtgref = msg;
  atomic_compiler_barrier();
  ch->ack = 0;
  tmc_mem_fence();
  atomic_compiler_barrier();
  ch->rdy = 1;
}

void *
ch_sym_ref_ad1_sm_no_receive(ch_sym_ref_ad1_sm_no_t *ch) {
  void *result = NULL;

  while (0 == ch->rdy)
    ;
  atomic_compiler_barrier();
  result = ch->vtgref;
  atomic_compiler_barrier();
  ch->rdy = 0;
  atomic_compiler_barrier();
  ch->ack = 1;

  return result;
}
