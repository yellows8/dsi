#ifdef USELIBNDS
#include <nds.h>
#else
#include <mininds/mininds.h>
#endif
#include <nds/ndstypes.h>
#include "twlescrypt.h"
#include "escryptipc.h"

static escryptipc_cmd escrypt_cmd;
extern char strbuf[];

static vu32 twlescrypt_arm7ready_flg = 0;

static int twlescrypt_sendmsg(escryptipc_cmd *cmd);

#ifndef NOIPC
static void twlescrypt_arm7readyhandler(u32 data, void* userdata)
{
	if(data==0x594452)
	{
		twlescrypt_arm7ready_flg = 1;//"RDY"
	}
}

static void twlescrypt_miniarm7readyhandler(u32 data)
{
	twlescrypt_arm7readyhandler(data, 0);
}

u32 twlescrypt_arm7ready()
{
	return twlescrypt_arm7ready_flg;
}
#endif

void twlescrypt_init()
{
	#ifndef NOIPC
	#ifdef USELIBNDS
	fifoSetValue32Handler(TWLESCRYPT_FIFOCHAN, twlescrypt_arm7readyhandler, 0);
	#else
	mininds_setfifochanhandler(TWLESCRYPT_FIFOCHAN, twlescrypt_miniarm7readyhandler);
	#endif
	while(!twlescrypt_arm7ready_flg)	
	{
		escrypt_cmd.cmdtype = 0xFF;
		twlescrypt_sendmsg(&escrypt_cmd);
		swiWaitForVBlank();
	}
	#endif
}

static int twlescrypt_sendmsg(escryptipc_cmd *cmd)
{
	int retval = 0;
	int counter = 0;
	#ifndef NOIPC
	cmd->busy = 1;
	DC_FlushRange(cmd, sizeof(escryptipc_cmd));
	#ifdef USELIBNDS
	fifoSendAddress(TWLESCRYPT_FIFOCHAN, cmd);
	#else
	mininds_sendfifodata(TWLESCRYPT_FIFOCHAN, (u32)cmd);
	#endif
	while(1)
	{
		if(counter>=10)
		{
			retval = -1;
			break;
		}
		DC_InvalidateRange(cmd, sizeof(escryptipc_cmd));
		if(cmd->busy==0)break;
		swiWaitForVBlank();
		counter++;
	}
	#endif
	return retval;
}

void dsi_set_key( dsi_context* ctx,
				 unsigned char key[16], unsigned char *keyX, unsigned char *keyY)
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_set_key;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)key;
	escrypt_cmd.args[2] = (u32)keyX;
	escrypt_cmd.args[3] = (u32)keyY;

	DC_FlushRange(ctx, sizeof(dsi_context));
	if(key)DC_FlushRange(key, 16);
	if(keyX)DC_FlushRange(keyX, 16);
	if(keyY)DC_FlushRange(keyY, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_add_ctr( dsi_context* ctx, unsigned char carry )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_add_ctr;
	
	escrypt_cmd.numargs = 2;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)carry;
	
	DC_FlushRange(ctx, sizeof(dsi_context));
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_set_ctr( dsi_context* ctx, unsigned char *ctr)
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_set_ctr;
	
	escrypt_cmd.numargs = 2;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)ctr;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(ctr, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_init_ctr( dsi_context* ctx,
				   unsigned char key[16], unsigned char *keyX, unsigned char *keyY,
				   unsigned char ctr[12] )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_init_ctr;
	
	escrypt_cmd.numargs = 5;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)key;
	escrypt_cmd.args[2] = (u32)keyX;
	escrypt_cmd.args[3] = (u32)keyY;
	escrypt_cmd.args[4] = (u32)ctr;

	DC_FlushRange(ctx, sizeof(dsi_context));
	if(key)DC_FlushRange(key, 16);
	if(keyX)DC_FlushRange(keyX, 16);
	if(keyY)DC_FlushRange(keyY, 16);
	DC_FlushRange(ctr, 12);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_crypt_ctr_block( dsi_context* ctx, 
								 unsigned char input[16], 
								 unsigned char output[16] )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_crypt_ctr_block;
	
	escrypt_cmd.numargs = 3;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)input;
	escrypt_cmd.args[2] = (u32)output;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(input, 16);
	DC_FlushRange(output, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	DC_InvalidateRange(output, 16);
}

void dsi_init_ccm( dsi_context* ctx,
				   unsigned char key[16], unsigned char *keyX, unsigned char *keyY,
				   unsigned int maclength,
				   unsigned int payloadlength,
				   unsigned int assoclength,
				   unsigned char nonce[12] )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_init_ccm;
	
	escrypt_cmd.numargs = 8;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)key;
	escrypt_cmd.args[2] = (u32)keyX;
	escrypt_cmd.args[3] = (u32)keyY;
	escrypt_cmd.args[4] = (u32)maclength;
	escrypt_cmd.args[5] = (u32)payloadlength;
	escrypt_cmd.args[6] = (u32)assoclength;
	escrypt_cmd.args[7] = (u32)nonce;

	DC_FlushRange(ctx, sizeof(dsi_context));
	if(key)DC_FlushRange(key, 16);
	if(keyX)DC_FlushRange(keyX, 16);
	if(keyY)DC_FlushRange(keyY, 16);
	DC_FlushRange(nonce, 12);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

#ifdef AESENC
void dsi_encrypt_ccm_block( dsi_context* ctx, 
								   unsigned char input[16], 
								   unsigned char output[16],
								   unsigned char* mac )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_encrypt_ccm_block;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)input;
	escrypt_cmd.args[2] = (u32)output;
	escrypt_cmd.args[3] = (u32)mac;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(input, 16);
	DC_FlushRange(output, 16);
	DC_FlushRange(mac, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	DC_InvalidateRange(output, 16);
	DC_InvalidateRange(mac, 16);
}
#endif

int dsi_decrypt_ccm_block( dsi_context* ctx, 
								   unsigned char input[16], 
								   unsigned char output[16],
								   unsigned char* mac )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_decrypt_ccm_block;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)input;
	escrypt_cmd.args[2] = (u32)output;
	escrypt_cmd.args[3] = (u32)mac;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(input, 16);
	DC_FlushRange(output, 16);
	DC_FlushRange(mac, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	DC_InvalidateRange(output, 16);
	DC_InvalidateRange(mac, 16);
	return escrypt_cmd.retval;
}

int dsi_decrypt_ccm( dsi_context* ctx, 
								   unsigned char input[16], 
								   unsigned char output[16],
								   unsigned int size,
								   unsigned char* mac )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_decrypt_ccm;
	
	escrypt_cmd.numargs = 5;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)input;
	escrypt_cmd.args[2] = (u32)output;
	escrypt_cmd.args[3] = (u32)size;
	escrypt_cmd.args[4] = (u32)mac;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(input, 16);
	DC_FlushRange(output, 16);
	DC_FlushRange(mac, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	DC_InvalidateRange(output, 16);
	DC_InvalidateRange(mac, 16);
	return escrypt_cmd.retval;
}

#ifdef AESENC
void dsi_encrypt_ccm( dsi_context* ctx, 
								   unsigned char input[16], 
								   unsigned char output[16],
								   unsigned int size,
								   unsigned char* mac )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_encrypt_ccm;
	
	escrypt_cmd.numargs = 5;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)input;
	escrypt_cmd.args[2] = (u32)output;
	escrypt_cmd.args[3] = (u32)size;
	escrypt_cmd.args[4] = (u32)mac;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(input, 16);
	DC_FlushRange(output, 16);
	DC_FlushRange(mac, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	DC_InvalidateRange(output, 16);
	DC_InvalidateRange(mac, 16);
}
#endif

void dsi_es_init( dsi_es_context* ctx,
				 unsigned char key[16], unsigned char *keyX, unsigned char *keyY)
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_es_init;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)key;
	escrypt_cmd.args[2] = (u32)keyX;
	escrypt_cmd.args[3] = (u32)keyY;

	DC_FlushRange(ctx, sizeof(dsi_context));
	if(key)DC_FlushRange(key, 16);
	if(keyX)DC_FlushRange(keyX, 16);
	if(keyY)DC_FlushRange(keyY, 16);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_es_set_nonce( dsi_es_context* ctx, unsigned char *nonce)
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_es_set_nonce;
	
	escrypt_cmd.numargs = 2;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)nonce;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(nonce, 12);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

void dsi_es_set_random_nonce( dsi_es_context* ctx)
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_es_set_random_nonce;
	
	escrypt_cmd.numargs = 1;
	escrypt_cmd.args[0] = (u32)ctx;

	DC_FlushRange(ctx, sizeof(dsi_context));
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}

int dsi_es_decrypt( dsi_es_context* ctx,
						    unsigned char* buffer,
						    unsigned char metablock[32],
							unsigned int size )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_es_decrypt;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)buffer;
	escrypt_cmd.args[2] = (u32)metablock;
	escrypt_cmd.args[3] = (u32)size;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(buffer, size);
	DC_FlushRange(metablock, 32);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(buffer, size);
	DC_InvalidateRange(metablock, 32);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
	return escrypt_cmd.retval;
}

#ifdef AESENC
void dsi_es_encrypt( dsi_es_context* ctx,
						    unsigned char* buffer,
						    unsigned char metablock[32],
							unsigned int size )
{
	memset(&escrypt_cmd, 0, sizeof(escryptipc_cmd));
	escrypt_cmd.cmdtype = CMD_dsi_es_encrypt;
	
	escrypt_cmd.numargs = 4;
	escrypt_cmd.args[0] = (u32)ctx;
	escrypt_cmd.args[1] = (u32)buffer;
	escrypt_cmd.args[2] = (u32)metablock;
	escrypt_cmd.args[3] = (u32)size;

	DC_FlushRange(ctx, sizeof(dsi_context));
	DC_FlushRange(buffer, size);
	DC_FlushRange(metablock, 32);
	twlescrypt_sendmsg(&escrypt_cmd);
	DC_InvalidateRange(buffer, size);
	DC_InvalidateRange(metablock, 32);
	DC_InvalidateRange(ctx, sizeof(dsi_context));
}
#endif

