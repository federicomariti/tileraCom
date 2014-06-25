#include "commons.h"


typedef struct {
  volatile int		rdy[MAX_CPU];
  void *		vtg[MAX_CPU];
  int			last_rcved;
  char			padding[BLOCK_SIZE - WORD_SIZE];

} in_rw_t;

typedef struct {
  volatile int		ack;
  char			padding[BLOCK_SIZE - WORD_SIZE];

} out_rw_t;

struct ch_asymin_ref_ad1_sm_param_b_t
{
  int			cpu_snd[MAX_CPU];
  volatile out_rw_t *	out_ref[MAX_CPU];
  int			cpu_rcv;
  int			num_multi;
  volatile in_rw_t *	in_ref;
  char			padding[BLOCK_SIZE - 3*WORD_SIZE];
};
