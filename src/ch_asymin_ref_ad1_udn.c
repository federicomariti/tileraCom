#include <stdlib.h>
#include <errno.h>
#include <tmc/udn.h>
#include <tmc/cpus.h>
#include <tmc/alloc.h>

#include "ch_asymin_ref_ad1_udn.h"
#include "ch_asymin_ref_ad1_udn_private.h"

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_2(int cpu_rcv, int cpu_snd[], param_t *param)
{
  return ch_asymin_ref_ad1_udn_create(cpu_rcv, cpu_snd, param->dq_rcv, param->dq_snd);
}

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create(int cpu_rcv, int cpu_snd[], int dq_rcv,
			     int dq_snd)
{
  ch_asymin_ref_ad1_udn_t *	result = NULL;
  tmc_alloc_t			alloc_ch = TMC_ALLOC_INIT;
  int				i;

  if (-1 == tmc_udn_activate()) return NULL;

  if (NULL==(result=tmc_alloc_map(&alloc_ch,sizeof(ch_asymin_ref_ad1_udn_t))))
    return NULL;

  result->cpu_rcv = cpu_rcv;
  result->dh_rcv = tmc_udn_header_from_cpu(cpu_rcv);
  result->dq_rcv = dq_rcv;
  result->dq_tag_rcv = __UDN_REG_TO_TAG(dq_rcv);
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
  switch (dq_snd) { // DEPR
  case 0:
    result->rcv_from_rcv = tmc_udn0_receive; break;
  case 1:
    result->rcv_from_rcv = tmc_udn1_receive; break;
  case 2:
    result->rcv_from_rcv = tmc_udn2_receive; break;
  case 3:
    result->rcv_from_rcv = tmc_udn3_receive; break;
  }
  result->dq_snd = dq_snd; // DEPR
  result->dq_tag_snd = __UDN_REG_TO_TAG(dq_snd); // DEPR

  for (i=0; i < MAX_CPU && -1 != cpu_snd[i]; i++) {
    result->cpu_snd[i] = cpu_snd[i];
    result->dh_snd[i] = tmc_udn_header_from_cpu(cpu_snd[i]);
    result->dq_snd_a[i] = dq_snd; // NEW
    result->dq_tag_snd_a[i] = __UDN_REG_TO_TAG(dq_snd); // NEW
    switch (dq_snd) { // NEW
    case 0:
      result->rcv_from_rcv_a[i] = tmc_udn0_receive; break;
    case 1:
      result->rcv_from_rcv_a[i] = tmc_udn1_receive; break;
    case 2:
      result->rcv_from_rcv_a[i] = tmc_udn2_receive; break;
    case 3:
      result->rcv_from_rcv_a[i] = tmc_udn3_receive; break;
    }      
    /* send first ack from receiver to sender i-th:
     * --> it is the RANK i of the sender i-th */
    tmc_udn_send_1(result->dh_snd[i], result->dq_tag_snd, (uint_reg_t)i);
  }
  if (MAX_CPU == i) { errno = EINVAL; return NULL; }

  return result;
}
/*
ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_2(int cpu_rcv, int cpu_snd[], int dq_rcv,
			       int dq_snd[])
{
  ch_asymin_ref_ad1_udn_t *result = NULL;
  return result;
}

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_3(int cpu_rcv, int cpu_snd[], int dq_rcv, 
			       int dq_snd, int num_cpu_snd)
{
  ch_asymin_ref_ad1_udn_t *result = NULL;
  return result;
}

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_4(int cpu_rcv, int cpu_snd[], int dq_rcv,
			       int dq_snd[], int num_cpu_snd)
{
  ch_asymin_ref_ad1_udn_t *result = NULL;
  return result;
}
*/
void
ch_asymin_ref_ad1_udn_destroy(ch_asymin_ref_ad1_udn_t **ch)
{
  tmc_alloc_unmap(*ch, sizeof(ch_asymin_ref_ad1_udn_t));
  *ch = NULL;
}

void
ch_asymin_ref_ad1_udn_send_2(ch_asymin_ref_ad1_udn_t *ch, void *msg)
{
  register uint_reg_t my_rank =  ch->rcv_from_rcv();

  tmc_udn_send_2(ch->dh_rcv, ch->dq_tag_rcv, (uint_reg_t) my_rank,
		 (uint_reg_t) msg);
}

void
ch_asymin_ref_ad1_udn_send(ch_asymin_ref_ad1_udn_t *ch, void *msg, int rank)
{
  (void)ch->rcv_from_rcv_a[rank]();
  tmc_udn_send_2(ch->dh_rcv, ch->dq_tag_rcv, (uint_reg_t)rank, (uint_reg_t)msg);
}

void *
ch_asymin_ref_ad1_udn_receive(ch_asymin_ref_ad1_udn_t *ch)
{
  register int sender_rank;
  register void *result = NULL;

  sender_rank = (int)ch->rcv_from_snd();
  result = (void *)ch->rcv_from_snd();
  tmc_udn_send_1(ch->dh_snd[sender_rank], ch->dq_tag_snd_a[sender_rank],
		 sender_rank /* per compatibilita` vecchia impl */);
  
  return result;
}
