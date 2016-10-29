#ifdef USELIBNDS
#include <nds.h>
#else
#include <mininds/mininds.h>
#endif
#include <nds/ndstypes.h>
#include "twlescrypt.h"
#include "escryptipc.h"

#define dsictx dsi_context

static u32 twlescrypt_arm9ready = 0;

#ifndef NOIPC
static void twlescrypthandler(void * address, void * userdata)
{
	escryptipc_cmd *cmd = (escryptipc_cmd*)address;
	u32 *args = cmd->args;
	switch(cmd->cmdtype)
	{
		case 0xFF:
			twlescrypt_arm9ready = 1;
			#ifdef USELIBNDS
			fifoSendValue32(TWLESCRYPT_FIFOCHAN,0x594452);
			#endif
		break;

		case CMD_dsi_set_key:
			dsi_set_key((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[3]);
		break;

		case CMD_dsi_add_ctr:
			dsi_add_ctr((dsictx*)args[0], (u8)args[1]);
		break;

		case CMD_dsi_set_ctr:
			dsi_set_ctr((dsictx*)args[0], (u8*)args[1]);
		break;

		case CMD_dsi_init_ctr:
			dsi_init_ctr((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[2], (u8*)args[3]);
		break;

		case CMD_dsi_crypt_ctr_block:
			dsi_crypt_ctr_block((dsictx*)args[0], (u8*)args[1], (u8*)args[2]);
		break;

		case CMD_dsi_init_ccm:
			dsi_init_ccm((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[3], (u32)args[4], (u32)args[5], (u32)args[6], (u8*)args[7]);
		break;

		#ifdef AESENC
		case CMD_dsi_encrypt_ccm_block:
			dsi_encrypt_ccm_block((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[3]);
		break;
		#endif

		case CMD_dsi_decrypt_ccm_block:
			cmd->retval = dsi_decrypt_ccm_block((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[3]);
		break;

		case CMD_dsi_decrypt_ccm:
			cmd->retval = dsi_decrypt_ccm((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u32)args[3], (u8*)args[4]);
		break;

		#ifdef AESENC
		case CMD_dsi_encrypt_ccm:
			dsi_encrypt_ccm((dsictx*)args[0], (u8*)args[1], (u8*)args[2], (u32)args[3], (u8*)args[4]);
		break;
		#endif

		case CMD_dsi_es_init:
			dsi_es_init((dsi_es_context*)args[0], (u8*)args[1], (u8*)args[2], (u8*)args[3]);
		break;

		case CMD_dsi_es_set_nonce:
			dsi_es_set_nonce((dsi_es_context*)args[0], (u8*)args[1]);
		break;

		case CMD_dsi_es_set_random_nonce:
			dsi_es_set_random_nonce((dsi_es_context*)args[0]);
		break;

		case CMD_dsi_es_decrypt:
			cmd->retval = dsi_es_decrypt((dsi_es_context*)args[0], (u8*)args[1], (u8*)args[2], (u32)args[3]);
		break;

		#ifdef AESENC
		case CMD_dsi_es_encrypt:
			dsi_es_encrypt((dsi_es_context*)args[0], (u8*)args[1], (u8*)args[2], (u32)args[3]);
		break;
		#endif
	}
	
	cmd->busy = 0;
}

static void mini_twlescrypthandler(u32 data)
{
	twlescrypthandler((void*)data, 0);
}
#endif

void twlescrypt_init()
{
	#ifndef NOIPC
	twlescrypt_arm9ready = 0;
	#ifdef USELIBNDS
	fifoSetAddressHandler(TWLESCRYPT_FIFOCHAN, twlescrypthandler, 0);
	#else
	mininds_setfifochanhandler(TWLESCRYPT_FIFOCHAN, mini_twlescrypthandler);
	#endif

	#ifndef USELIBNDS
	while(!twlescrypt_arm9ready)
	{
		mininds_sendfifodata(TWLESCRYPT_FIFOCHAN,0x594452);//"RDY"
		swiWaitForVBlank();
	}
	#endif

	#endif
}

