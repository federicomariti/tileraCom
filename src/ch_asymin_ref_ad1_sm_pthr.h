#ifndef __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_PTHR_H__
#define __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_PTHR_H__

typedef struct ch_asymin_ref_ad1_sm_pthr_t ch_asymin_ref_ad1_sm_pthr_t;

ch_asymin_ref_ad1_sm_pthr_t *
ch_asymin_ref_ad1_sm_pthr_create(int cpu_rcv, int cpu_snd[]);

void
ch_asymin_ref_ad1_sm_pthr_destroy(ch_asymin_ref_ad1_sm_pthr_t **ch);

void
ch_asymin_ref_ad1_sm_pthr_send(ch_asymin_ref_ad1_sm_pthr_t *ch, void *msg);

void *
ch_asymin_ref_ad1_sm_pthr_receive(ch_asymin_ref_ad1_sm_pthr_t *ch);

#endif /* __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_PTHR_H__ */
