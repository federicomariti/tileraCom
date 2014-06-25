#include "commons.h"


struct ch_sym_ref_ad1_sm_no_t
{
  int cpu_snd;
  int cpu_rcv;
  volatile int rdy;
  volatile int ack;
  void **vtgref;
  int vtg_is_alloc;
  char padding[BLOCK_SIZE - 6 * WORD_SIZE];
};
