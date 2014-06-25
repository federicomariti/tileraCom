#include "commons.h"

/**
 * Descrittore del canale di comunicazione
 */
struct ch_sym_ref_ad1_udn_t
{
  int		cpu_snd;
  int		cpu_rcv;
  int		dq_snd;
  int		dq_rcv;
  uint32_t	dq_tag_snd;
  uint32_t	dq_tag_rcv;
  DynamicHeader dh_snd;
  DynamicHeader dh_rcv;
  uint_reg_t (*	rcv_from_rcv)(void);
  uint_reg_t (*	rcv_from_snd)(void);
  char		padding[BLOCK_SIZE - 11*WORD_SIZE];
};
