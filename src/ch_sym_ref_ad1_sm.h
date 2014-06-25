#ifndef __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_H__
#define __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_H__

/**
 * Definizione del tipo che rappresenta un canale di comunicazione
 * simmetrico con grado di asincronia 1. Tale struttura viene detta
 * descrittore del canale di comunicazione ed e` condivisa tra i due
 * processi comunicanti.
 */
typedef struct ch_sym_ref_ad1_sm_t ch_sym_ref_ad1_sm_t;

#ifdef CHANNEL_SIMMETRIC_REF_AD1_SM__TEST
int csra1_sm_test_structureSize(void);
#endif 

#ifdef CHANNEL_SIMMETRIC_REF_AD1_SM__DEBUG
#include <stdio.h>
void csra1_sm_debug_printStructuresSize(void);
#endif 

/**
 * Alloca ed inizializza le strutture dati necessarie al supporto del
 * canale.
 *
 * Ignorato vtg_ref (attualmente)
 */
ch_sym_ref_ad1_sm_t *
ch_sym_ref_ad1_sm_create(int cpu_snd, int cpu_rcv, void **vtg_ref);

/**
 * Dealloca tutte le strutture allocate a supporto del canale
 */
void
ch_sym_ref_ad1_sm_destroy(ch_sym_ref_ad1_sm_t **ch);

/**
 * 
 */
void
ch_sym_ref_ad1_sm_send(ch_sym_ref_ad1_sm_t *ch, void *msg);

/**
 *
 */
void *
ch_sym_ref_ad1_sm_receive(ch_sym_ref_ad1_sm_t *ch);


#endif /* __CHANNEL_SIMMETRIC_REFERENCE_AD1_SM_H__ */
