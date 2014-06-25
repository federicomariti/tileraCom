#ifndef __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_2_H__
#define __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_2_H__

typedef struct ch_asymin_ref_ad1_sm_param_t ch_asymin_ref_ad1_sm_param_t;

ch_asymin_ref_ad1_sm_param_t *
ch_asymin_ref_ad1_sm_param_create_2(int cpu_rcv, int cpu_snd[], void *param);

ch_asymin_ref_ad1_sm_param_t *
ch_asymin_ref_ad1_sm_param_create(int cpu_rcv, int cpu_snd[]);

ch_asymin_ref_ad1_sm_param_t *
ch_asymin_ref_ad1_sm_param_init_sender(ch_asymin_ref_ad1_sm_param_t * ch);

void
ch_asymin_ref_ad1_sm_param_destroy(ch_asymin_ref_ad1_sm_param_t **ch);

void
ch_asymin_ref_ad1_sm_param_send(ch_asymin_ref_ad1_sm_param_t *ch, void *msg, int cpu);

void *
ch_asymin_ref_ad1_sm_param_receive(ch_asymin_ref_ad1_sm_param_t *ch);

#endif /* __CHANNEL_ASIMMETRIC_OUT_REF_AD1_SM_2_H__ */
