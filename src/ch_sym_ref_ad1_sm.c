#include <stdlib.h>
#include <tmc/alloc.h>
#include <arch/atomic.h>

#include "ch_sym_ref_ad1_sm.h"

/**
 * Dimensione di una pagina della gerarchia di memoria L2 cache - main
 * memory
 */
#define PAGE_SIZE 65536
/**
 * Dimensione in byte di una parola (intero o riferimento)
 */
#define WORD_SIZE 4

typedef struct {
  volatile int	rdy;
  void *	vtg;
  char		padding[PAGE_SIZE - 2 * WORD_SIZE];

} in_rw_t;

typedef struct {
  volatile int	ack;
  char		padding[PAGE_SIZE - WORD_SIZE];

} out_rw_t;

struct ch_sym_ref_ad1_sm_t
{
  int		cpu_snd;
  int		cpu_rcv;
  in_rw_t *	in_rw;
  out_rw_t *	out_rw;
  //void **	vtgref;
  char		padding[PAGE_SIZE - 4 * WORD_SIZE];
};

#define ch_rdy in_rw->rdy
#define ch_ack out_rw->ack
#define ch_vtg in_rw->vtg

ch_sym_ref_ad1_sm_t *
ch_sym_ref_ad1_sm_create(int cpu_snd, int cpu_rcv, void **vtgref)
{
  ch_sym_ref_ad1_sm_t *result = NULL;
  tmc_alloc_t alloc_descr = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_snd = TMC_ALLOC_INIT;
  tmc_alloc_t alloc_rcv = TMC_ALLOC_INIT;

  tmc_alloc_set_home(&alloc_snd, cpu_snd);
  tmc_alloc_set_home(&alloc_rcv, cpu_rcv);

  if (NULL == (result = tmc_alloc_map(&alloc_descr,
				      sizeof(ch_sym_ref_ad1_sm_t))))
    return NULL;
  if (NULL == (result->in_rw = tmc_alloc_map(&alloc_rcv, sizeof(in_rw_t))))
    return NULL;
  if (NULL == (result->out_rw = tmc_alloc_map(&alloc_snd, sizeof(out_rw_t))))
    return NULL;

  result->cpu_snd = cpu_snd;
  result->cpu_rcv = cpu_rcv;
  result->ch_rdy = 0;
  result->ch_ack = 1;

  return result;
}

void
ch_sym_ref_ad1_sm_destroy(ch_sym_ref_ad1_sm_t **ch)
{
  tmc_alloc_unmap((*ch)->in_rw, sizeof(in_rw_t));
  tmc_alloc_unmap((*ch)->out_rw, sizeof(out_rw_t));
  tmc_alloc_unmap(*ch, sizeof(ch_sym_ref_ad1_sm_t));
  *ch = NULL;
}

void
ch_sym_ref_ad1_sm_send(ch_sym_ref_ad1_sm_t *ch,
				  void *msg)
{
  while (0 == ch->ch_ack)
    ;
  atomic_compiler_barrier();
  ch->ch_vtg = msg;
  atomic_compiler_barrier();
  ch->ch_ack = 0;
  atomic_compiler_barrier();
  ch->ch_rdy = 1;
}

void *
ch_sym_ref_ad1_sm_receive(ch_sym_ref_ad1_sm_t *ch)
{
  void *result = NULL;

  while (0 == ch->ch_rdy)
    ;
  atomic_compiler_barrier();
  result = ch->ch_vtg;
  atomic_compiler_barrier();
  ch->ch_rdy = 0;
  atomic_compiler_barrier();
  ch->ch_ack = 1;

  return result;
}
