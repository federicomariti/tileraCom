#include "commons.h"

typedef struct {
  volatile void *	vtg;
  char			padding[BLOCK_SIZE - WORD_SIZE];

} in_rw_t;

typedef struct {
  volatile int	ack;
  char		padding[BLOCK_SIZE - WORD_SIZE];

} out_rw_t;

struct ch_sym_ref_ad1_sm_nullack_t
{
  int			cpu_snd;
  int			cpu_rcv;
  volatile in_rw_t *	in_ref;
  volatile out_rw_t *	out_ref;
  char			padding[BLOCK_SIZE - 4 * WORD_SIZE];
};
