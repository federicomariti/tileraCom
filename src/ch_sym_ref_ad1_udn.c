#include <stdlib.h>
#include <tmc/udn.h>
#include <tmc/alloc.h>
#include <arch/udn.h>

#include "ch_sym_ref_ad1_udn.h"
#include "ch_sym_ref_ad1_udn_private.h"

#define ACK_WORD 1

int
csra1_udn_test_structureSize(void)
{
  return ! sizeof(ch_sym_ref_ad1_udn_t) == BLOCK_SIZE;
}

ch_sym_ref_ad1_udn_t *
ch_sym_ref_ad1_udn_create_2(int cpu_snd, int cpu_rcv, const ch_sym_ref_ad1_udn_param_t *param)
{
  return ch_sym_ref_ad1_udn_create(cpu_snd, cpu_rcv, param->dq_snd, param->dq_rcv);
}

ch_sym_ref_ad1_udn_t *
ch_sym_ref_ad1_udn_create(int cpu_snd, int cpu_rcv, int dq_snd, int dq_rcv)
{
  ch_sym_ref_ad1_udn_t *	result = NULL;
  tmc_alloc_t			alloc_snd = TMC_ALLOC_INIT;

  if (-1 == tmc_udn_activate()) return NULL;

  tmc_alloc_set_home(&alloc_snd, cpu_snd);
  if (NULL == (result = tmc_alloc_map(&alloc_snd,
				      sizeof(ch_sym_ref_ad1_udn_t))))
    return NULL;
  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->dq_snd = dq_snd;
  result->dq_rcv = dq_rcv;
  result->dq_tag_snd = __UDN_REG_TO_TAG(dq_snd);
  result->dq_tag_rcv = __UDN_REG_TO_TAG(dq_rcv);
  result->dh_snd = tmc_udn_header_from_cpu(cpu_snd);
  result->dh_rcv = tmc_udn_header_from_cpu(cpu_rcv);
  switch (dq_snd) {
  case 0:
    result->rcv_from_rcv = tmc_udn0_receive; break;
  case 1:
    result->rcv_from_rcv = tmc_udn1_receive; break;
  case 2:
    result->rcv_from_rcv = tmc_udn2_receive; break;
  case 3:
    result->rcv_from_rcv = tmc_udn3_receive; break;
  }
  switch (dq_rcv) {
  case 0:
    result->rcv_from_snd = tmc_udn0_receive; break;
  case 1:
    result->rcv_from_snd = tmc_udn1_receive; break;
  case 2:
    result->rcv_from_snd = tmc_udn2_receive; break;
  case 3:
    result->rcv_from_snd = tmc_udn3_receive; break;
  }

  /* first ack */
  tmc_udn_send_1(result->dh_snd, result->dq_tag_snd, ACK_WORD);

  return result;
}

void
ch_sym_ref_ad1_udn_destroy(ch_sym_ref_ad1_udn_t **ch)
{
  tmc_alloc_unmap(*ch, sizeof(ch_sym_ref_ad1_udn_t));
  *ch = NULL;
}

void
ch_sym_ref_ad1_udn_send(ch_sym_ref_ad1_udn_t *ch, void *msg)
{
  (void)ch->rcv_from_rcv();
  tmc_udn_send_1(ch->dh_rcv, ch->dq_tag_rcv, (uint_reg_t)msg);
}


void *
ch_sym_ref_ad1_udn_receive(ch_sym_ref_ad1_udn_t *ch)
{
  void *result = NULL;

  result = (void *)ch->rcv_from_snd();
  tmc_udn_send_1(ch->dh_snd, ch->dq_tag_snd, ACK_WORD);

  return result;
}

