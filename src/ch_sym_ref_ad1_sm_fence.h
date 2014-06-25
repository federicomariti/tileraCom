#ifndef __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_FENCE_H__
#define __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_FENCE_H__

/**
 * Definizione del tipo che rappresenta un canale di comunicazione
 * simmetrico con grado di asincronia 1. Tale struttura viene detta
 * descrittore del canale di comunicazione ed e` condivisa tra i due
 * processi comunicanti.
 */
typedef struct ch_sym_ref_ad1_sm_fence_t ch_sym_ref_ad1_sm_fence_t;

#ifdef CHANNEL_SIMMETRIC_REF_AD1_SM_FENCE__TEST
int csra1_sm_fence_test_structureSize(void);
#endif 

#ifdef CHANNEL_SIMMETRIC_REF_AD1_SM_FENCE__DEBUG
#include <stdio.h>
void csra1_sm_fence_debug_printStructuresSize(void);
#endif 

/**
 * Alloca ed inizializza le strutture dati necessarie al supporto del
 * canale.
 *
 * Ignorato vtg_ref (attualmente)
 */
ch_sym_ref_ad1_sm_fence_t *
ch_sym_ref_ad1_sm_fence_create(int cpu_snd, int cpu_rcv, void *param);

/**
 * Dealloca tutte le strutture allocate a supporto del canale
 */
void
ch_sym_ref_ad1_sm_fence_destroy(ch_sym_ref_ad1_sm_fence_t **ch);

void
ch_sym_ref_ad1_sm_fence_send(ch_sym_ref_ad1_sm_fence_t *ch, void *msg);

void *
ch_sym_ref_ad1_sm_fence_receive(ch_sym_ref_ad1_sm_fence_t *ch);


#endif /* __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_FENCE_H__ */
