#ifndef __CHANNEL_ASIMMETRIC_OUT_REF_AD1_UDN_H__
#define __CHANNEL_ASIMMETRIC_OUT_REF_AD1_UDN_H__

typedef struct ch_asymin_ref_ad1_udn_t ch_asymin_ref_ad1_udn_t;

typedef struct
{
  int dq_snd;
  int dq_rcv;

} param_t;

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_2(int cpu_rcv, int cpu_snd[], param_t *param);

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create(int cpu_rcv, int cpu_snd[], int dq_rcv,
			     int dq_snd);

/*
ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_2(int cpu_rcv, int cpu_snd[], int dq_rcv,
			       int dq_snd[]);

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_3(int cpu_rcv, int cpu_snd[], int dq_rcv, 
			       int dq_snd, int num_cpu_snd);

ch_asymin_ref_ad1_udn_t *
ch_asymin_ref_ad1_udn_create_4(int cpu_rcv, int cpu_snd[], int dq_rcv,
			       int dq_snd[], int num_cpu_snd);
*/
void
ch_asymin_ref_ad1_udn_destroy(ch_asymin_ref_ad1_udn_t **ch);

void
ch_asymin_ref_ad1_udn_send_2(ch_asymin_ref_ad1_udn_t *ch, void *msg);

void
ch_asymin_ref_ad1_udn_send(ch_asymin_ref_ad1_udn_t *ch, void *msg, int rank);

void *
ch_asymin_ref_ad1_udn_receive(ch_asymin_ref_ad1_udn_t *ch);

#endif /* __CHANNEL_ASIMMETRIC_OUT_REF_AD1_UDN_H__ */
