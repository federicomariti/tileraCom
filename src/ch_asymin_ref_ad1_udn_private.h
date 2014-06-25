#include "commons.h"


/**
 * Descrittore del canale di comunicazione
 */
struct ch_asymin_ref_ad1_udn_t
{
  int		cpu_rcv;
  DynamicHeader dh_rcv;
  int		dq_rcv;
  uint32_t	dq_tag_rcv;
  uint_reg_t (*	rcv_from_snd)(void);
  int		cpu_snd[MAX_CPU]; /* indirizzato per ide. del mittente
				     nel canale */
  DynamicHeader dh_snd[MAX_CPU]; /* indirizzato per ide. del mittente
				    nel canale */

  int		dq_snd; // DEPR
  uint32_t	dq_tag_snd; // DEPR
  uint_reg_t (*	rcv_from_rcv)(void); // DEPR

  int		dq_snd_a[MAX_CPU];
  uint32_t	dq_tag_snd_a[MAX_CPU];
  uint_reg_t (* rcv_from_rcv_a[MAX_CPU])(void);

  char		padding[BLOCK_SIZE - 8*WORD_SIZE];
};
