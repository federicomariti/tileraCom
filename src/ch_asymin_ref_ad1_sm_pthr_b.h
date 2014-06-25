#ifndef __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_1B_H__
#define __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_1B_H__

typedef struct ch_asymin_ref_ad1_sm_struct_b_t ch_asymin_ref_ad1_sm_struct_b_t;

ch_asymin_ref_ad1_sm_struct_b_t *
ch_asymin_ref_ad1_sm_struct_b_create(int cpu_rcv, int cpu_snd[]);

void
ch_asymin_ref_ad1_sm_struct_b_init_sender(ch_asymin_ref_ad1_sm_struct_b_t **ch);

void
ch_asymin_ref_ad1_sm_struct_b_destroy(ch_asymin_ref_ad1_sm_struct_b_t **ch);

void
ch_asymin_ref_ad1_sm_struct_b_send(ch_asymin_ref_ad1_sm_struct_b_t *ch, void *msg);

void *
ch_asymin_ref_ad1_sm_struct_b_receive(ch_asymin_ref_ad1_sm_struct_b_t *ch);

#endif /* __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_1B_H__ */
