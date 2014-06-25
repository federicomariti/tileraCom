#ifndef __CHANNEL_SIMMETRIC_REF_AD1_UDN_H__
#define __CHANNEL_SIMMETRIC_REF_AD1_UDN_H__

typedef struct ch_sym_ref_ad1_udn_t ch_sym_ref_ad1_udn_t;
typedef struct ch_sym_ref_ad1_udn_param_t
{
  int dq_snd;
  int dq_rcv;

} ch_sym_ref_ad1_udn_param_t;

#ifdef CHANNEL_SIMMETRIC_REF_AD1_UDN__TEST
int csra1_udn_test_structureSize(void);
#endif


ch_sym_ref_ad1_udn_t *
ch_sym_ref_ad1_udn_create(int cpu_snd, int cpu_rcv, int dq_snd, int dq_rcv);

ch_sym_ref_ad1_udn_t *
ch_sym_ref_ad1_udn_create_2(int cpu_snd, int cpu_rcv, const ch_sym_ref_ad1_udn_param_t *param);

void
ch_sym_ref_ad1_udn_destroy(ch_sym_ref_ad1_udn_t **ch);

void
ch_sym_ref_ad1_udn_send(ch_sym_ref_ad1_udn_t *ch, void *msg);

void *
ch_sym_ref_ad1_udn_receive(ch_sym_ref_ad1_udn_t *ch);

#endif /* __CHANNEL_SIMMETRIC_REF_AD1_UDN_H__ */
