#include "commons.h"


typedef struct {
  volatile void *	vtg;
  char			padding[BLOCK_SIZE - WORD_SIZE];

} in_rw_t;

struct ch_sym_ref_ad1_sm_null_t
{
  int			cpu_snd;
  int			cpu_rcv;
  volatile in_rw_t *	in_rw;
  volatile void *	vtg;
  char			padding[BLOCK_SIZE - 4 * WORD_SIZE];
};

