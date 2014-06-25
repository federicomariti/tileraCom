#ifndef __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_STRUCT_H__
#define __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_STRUCT_H__

typedef struct ch_asymin_ref_ad1_sm_struct_t ch_asymin_ref_ad1_sm_struct_t;

ch_asymin_ref_ad1_sm_struct_t *
ch_asymin_ref_ad1_sm_struct_create_2(int cpu_rcv, int cpu_snd[], void *param);

ch_asymin_ref_ad1_sm_struct_t *
ch_asymin_ref_ad1_sm_struct_create(int cpu_rcv, int cpu_snd[]);

void
ch_asymin_ref_ad1_sm_struct_init_sender(ch_asymin_ref_ad1_sm_struct_t **ch);

void
ch_asymin_ref_ad1_sm_struct_destroy(ch_asymin_ref_ad1_sm_struct_t **ch);

void
ch_asymin_ref_ad1_sm_struct_send(ch_asymin_ref_ad1_sm_struct_t *ch, void *msg);

void *
ch_asymin_ref_ad1_sm_struct_receive(ch_asymin_ref_ad1_sm_struct_t *ch);

#endif /* __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_STRUCT_H__ */
