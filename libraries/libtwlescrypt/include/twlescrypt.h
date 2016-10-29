#ifndef _H_TWLESCRYPT_
#define _H_TWLESCRYPT_

#ifndef ARM7
#ifndef ARM9
#error "Define either ARM7 or ARM9."
#endif
#endif

#include "aes.h"
#include "dsi.h"

#define CMD_dsi_set_key 0
#define CMD_dsi_add_ctr 1
#define CMD_dsi_set_ctr 2
#define CMD_dsi_init_ctr 3
#define CMD_dsi_crypt_ctr_block 4
#define CMD_dsi_init_ccm 5
#define CMD_dsi_encrypt_ccm_block 6
#define CMD_dsi_decrypt_ccm_block 7
#define CMD_dsi_decrypt_ccm 8
#define CMD_dsi_encrypt_ccm 9
#define CMD_dsi_es_init 10
#define CMD_dsi_es_set_nonce 11
#define CMD_dsi_es_set_random_nonce 12
#define CMD_dsi_es_decrypt 13
#define CMD_dsi_es_encrypt 14

void twlescrypt_init();

#endif

