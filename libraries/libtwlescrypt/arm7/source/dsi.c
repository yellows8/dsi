#include "dsi.h"
#include <nds/ndstypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum
{
	AES_KEYMODE_NORMAL = 0,
	AES_KEYMODE_X = 1,
	AES_KEYMODE_Y = 2
} AES_KeyMode;

void AES_LoadKey(u32 keyslot);
void AES_SetKey(u32 keyslot, AES_KeyMode mode, u32 key[4]);
void AES_SetCounter(u32 counter[4]);
void AES_CtrCrypt(u32* input, u32* output);
int AES_CCMEncrypt(u32* input, u32* output, unsigned int size, u32 *mac, u32 *nonce, u32 *S0);
int AES_CCMDecrypt(u32* input, u32* output, unsigned int numblocks, u32 *mac, u32 *nonce, u32 *S0);

void dsi_set_key( dsi_context* ctx,
				 unsigned char key[16], unsigned char *keyX, unsigned char *keyY)
{
	int i;
	unsigned char keyswap[16];
	unsigned int keyword[4];
	unsigned int keywordX[4];
	unsigned int keywordY[4];

	memcpy(keyword, key, 16);
	if(keyX)memcpy(keywordX, keyX, 16);
	if(keyY)memcpy(keywordY, keyY, 16);

	for(i=0; i<16; i++)
		keyswap[i] = key[15-i];

	//aes_setkey_enc(&ctx->aes, keyswap, 128);
	if(keyX==NULL && keyY==NULL)AES_SetKey(0, AES_KEYMODE_NORMAL, keyword);
	if(keyX)AES_SetKey(0, AES_KEYMODE_X, keywordX);
	if(keyY)AES_SetKey(0, AES_KEYMODE_Y, keywordY);
	AES_WaitKey();
}

void dsi_add_ctr( dsi_context* ctx,
				  unsigned char carry )
{
	unsigned char sum;
	int i;
	unsigned int ctr[4];

	/*for(i=15; i>=0; i--)
	{
		sum = ctx->ctr[i] + carry;

		if (sum < ctx->ctr[i])
			carry = 1;
		else
			carry = 0;

		ctx->ctr[i] = sum;
	}*/

	for(i=0; i<16; i++)((unsigned char*)ctr)[i] = ctx->ctr[i];
	AES_AddToCounter(ctr, carry);
	AES_SetCounter(ctr);
	memcpy(ctx->ctr, ctr, 16);
}
				  
void dsi_set_ctr( dsi_context* ctx,
				  unsigned char ctr[16] )
{
	int i;
	unsigned int wordctr[4];

	for(i=0; i<16; i++)
		ctx->ctr[i] = ctr[i];

	memcpy(wordctr, ctr, 16);
	AES_SetCounter(wordctr);
}

void dsi_init_ctr( dsi_context* ctx,
				   unsigned char key[16], unsigned char *keyX, unsigned char *keyY,
				   unsigned char ctr[12] )
{
	dsi_set_key(ctx, key, keyX, keyY);
	dsi_set_ctr(ctx, ctr);
}

void dsi_crypt_ctr_block( dsi_context* ctx, 
						  unsigned char input[16], 
						  unsigned char output[16] )
{
	int i;
	//unsigned char stream[16];
	unsigned int in[4];
	unsigned int out[4];
	unsigned int ctr[4];

	for(i=0; i<16; i++)((unsigned char*)ctr)[i] = ctx->ctr[i];

	memset(in, 0, 16);
	if(input)memcpy(in, input, 16);
	AES_Reset();
	AES_LoadKey(0);
	AES_WaitKey();
	AES_SetCounter(ctr);
	AES_CtrCrypt(in, out);
	memcpy(output, out, 16);

	/*aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->ctr, stream);


	if (input)
	{
		for(i=0; i<16; i++)
		{
			output[i] = stream[15-i] ^ input[i];
		}
	}
	else
	{
		for(i=0; i<16; i++)
			output[i] = stream[15-i];
	}*/

	dsi_add_ctr(ctx, 1);
}


void dsi_init_ccm( dsi_context* ctx,
				   unsigned char key[16], unsigned char *keyX, unsigned char *keyY,
				   unsigned int maclength,
				   unsigned int payloadlength,
				   unsigned int assoclength,
				   unsigned char nonce[12] )
{
	int i;



	dsi_set_key(ctx, key, keyX, keyY);

	ctx->maclen = maclength;

	maclength = (maclength-2)/2;

	payloadlength = (payloadlength+15) & ~15;

	// CCM B0 block:
	// [1-byte flags] [12-byte nonce] [3-byte size]
	ctx->mac[15] = (maclength<<3) | 2;
	if (assoclength)
		ctx->mac[0] |= (1<<6);
	for(i=0; i<12; i++)
		ctx->mac[3+i] = nonce[i];
	ctx->mac[2] = payloadlength>>16;
	ctx->mac[1] = payloadlength>>8;
	ctx->mac[0] = payloadlength>>0;

	//aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);
	memset(ctx->ctr, 0, 16);
	dsi_crypt_ctr_block(ctx, ctx->mac, ctx->mac);

	// CCM CTR:
	// [1-byte flags] [12-byte nonce] [3-byte ctr]
	ctx->ctr[15] = 2;
	for(i=0; i<12; i++)
		ctx->ctr[3+i] = nonce[i];//11-
	ctx->ctr[2] = (unsigned char)((payloadlength>>4)>>16);
	ctx->ctr[1] = (unsigned char)((payloadlength>>4)>>8);
	ctx->ctr[0] = (unsigned char)(payloadlength>>4);

	dsi_crypt_ctr_block(ctx, 0, ctx->S0);
}

#ifdef AESENC
void dsi_encrypt_ccm_block( dsi_context* ctx, 
							unsigned char input[16], 
							unsigned char output[16],
							unsigned char* mac )
{
	int i;
	unsigned int ctr[4];
	unsigned int S0[4];
	memcpy(S0, ctx->S0, 16);
	memset(ctr, 0, 16);
	for(i=0; i<12; i++)((unsigned char*)ctr)[i] = ctx->ctr[i+3];//15-

	AES_CCMEncrypt((unsigned int*)input, (unsigned int*)output, 16, (unsigned int*)mac, ctr, S0);

	/*int i;

	for(i=0; i<16; i++)
		ctx->mac[i] ^= input[15-i];

	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);

	if (mac)
	{
		for(i=0; i<16; i++)
			mac[i] = ctx->mac[15-i] ^ ctx->S0[i];
	}

	if (output)
		dsi_crypt_ctr_block(ctx, input, output);*/
}
#endif


int dsi_decrypt_ccm_block( dsi_context* ctx, 
							unsigned char input[16], 
							unsigned char output[16],
							unsigned char* mac )
{
	int i;
	unsigned int ctr[4];
	unsigned int S0[4];
	memcpy(S0, ctx->S0, 16);
	memset(ctr, 0, 16);
	for(i=0; i<12; i++)((unsigned char*)ctr)[i] = ctx->ctr[i+3];//15-

	return AES_CCMDecrypt((unsigned int*)input, (unsigned int*)output, 16, (unsigned int*)mac, ctr, S0);

	/*int i;


	if (output)
	{
		dsi_crypt_ctr_block(ctx, input, output);


		for(i=0; i<16; i++)
			ctx->mac[i] ^= output[15-i];
	}
	else
	{
		for(i=0; i<16; i++)
			ctx->mac[i] ^= input[15-i];
	}

	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->mac, ctx->mac);


	if (mac)
	{
		for(i=0; i<16; i++)
			mac[i] = ctx->mac[15-i] ^ ctx->S0[i];
	}
	return 0;*/
}


int dsi_decrypt_ccm( dsi_context* ctx, 
					  unsigned char* input, 
					  unsigned char* output,
					  unsigned int size,
					  unsigned char* mac )
{
	int i, retval;
	unsigned int nonce[4];
	unsigned int macword[4];
	unsigned int S0[4];
	memset(nonce, 0, 16);
	for(i=0; i<12; i++)((unsigned char*)nonce)[i] = ctx->ctr[i+3];//15-
	memcpy(macword, mac, 16);
	memcpy(S0, ctx->S0, 16);

	retval = AES_CCMDecrypt((unsigned int*)input, (unsigned int*)output, size, macword, nonce, S0);
	return retval;

	/*unsigned char block[16];
	unsigned char ctr[16];

	while(size > 16)
	{
		dsi_decrypt_ccm_block(ctx, input, output, mac);

		if (input)
			input += 16;
		if (output)
			output += 16;

		size -= 16;
	}

	memcpy(ctr, ctx->ctr, 16);
	memset(block, 0, 16);	
	dsi_crypt_ctr_block(ctx, block, block);
	memcpy(ctx->ctr, ctr, 16);
	memcpy(block, input, size);


	dsi_decrypt_ccm_block(ctx, block, block, mac);
	memcpy(output, block, size);*/
}


#ifdef AESENC
void dsi_encrypt_ccm( dsi_context* ctx, 
					  unsigned char* input, 
					  unsigned char* output,
					  unsigned int size,
					  unsigned char* mac )
{
	int i;
	unsigned int nonce[4];
	unsigned int macword[4];
	unsigned int S0[4];
	memset(nonce, 0, 16);
	memset(macword, 0, 16);
	for(i=0; i<12; i++)((unsigned char*)nonce)[i] = ctx->ctr[i+3];//15-
	memcpy(S0, ctx->S0, 16);

	AES_CCMEncrypt((unsigned int*)input, (unsigned int*)output, size, macword, nonce, ctx->S0);
	memcpy(mac, macword, 16);

	/*unsigned char block[16];

	while(size > 16)
	{
		dsi_encrypt_ccm_block(ctx, input, output, mac);

		if (input)
			input += 16;
		if (output)
			output += 16;

		size -= 16;
	}

	size-= size/16;

	memset(block, 0, 16);
	memcpy(block, input, size);
	dsi_encrypt_ccm_block(ctx, block, block, mac);
	memcpy(output, block, size);*/
}
#endif

void dsi_es_init( dsi_es_context* ctx,
				  unsigned char key[16], unsigned char *keyX, unsigned char *keyY )
{
	ctx->keyflags = 0;
	if(keyX)ctx->keyflags |= 1;
	if(keyY)ctx->keyflags |= 2;

	memcpy(ctx->key, key, 16);
	if(keyX)memcpy(ctx->keyX, keyX, 16);
	if(keyY)memcpy(ctx->keyY, keyY, 16);
	ctx->randomnonce = 1;
}

void dsi_es_set_nonce( dsi_es_context* ctx,
					   unsigned char nonce[12] )
{
	memcpy(ctx->nonce, nonce, 12);
	ctx->randomnonce = 0;
}

void dsi_es_set_random_nonce( dsi_es_context* ctx )
{
	ctx->randomnonce = 1;
}
							 

int dsi_es_decrypt( dsi_es_context* ctx,
				    unsigned char* buffer,
				    unsigned char metablock[32],
					unsigned int size )
{
	unsigned char ctr[16];
	unsigned char nonce[12];
	unsigned char scratchpad[16];
	unsigned char chkmac[16];
	unsigned char genmac[16];
	dsi_context cryptoctx;
	unsigned int chksize;
	unsigned char *keyX = NULL, *keyY = NULL;

	memcpy(chkmac, metablock, 16);

	memcpy(ctr, metablock + 16, 16);
	ctr[0] = 0;
	ctr[13] = 0;
	ctr[14] = 0;
	ctr[15] = 0;

	if(ctx->keyflags & 1)keyX = ctx->keyX;
	if(ctx->keyflags & 2)keyY = ctx->keyY;
	dsi_init_ctr(&cryptoctx, ctx->key, keyX, keyY, ctr);
	dsi_crypt_ctr_block(&cryptoctx, metablock+16, scratchpad);

	chksize = (scratchpad[13]<<16) | (scratchpad[14]<<8) | (scratchpad[15]<<0);

	if (scratchpad[0] != 0x3A || chksize != size)
	{
		/*memset(buffer, 0, size);
		memcpy(buffer, ctr, 16);
		memcpy(&buffer[0x10], metablock, 0x20);
		memcpy(&buffer[0x30], scratchpad, 0x10);*/
		return -1;
		//return 1;
	}

	memcpy(nonce, metablock + 17, 12);

	dsi_init_ccm(&cryptoctx, ctx->key, keyX, keyY, 16, size, 0, nonce);
	if(dsi_decrypt_ccm(&cryptoctx, buffer, buffer, size, chkmac)!=0)
	{
		return 1;
		//return -2;
	}

	//if (memcmp(genmac, chkmac, 16) != 0)
	//	return -1;

	return 0;
}


#ifdef AESENC
void dsi_es_encrypt( dsi_es_context* ctx,
				     unsigned char* buffer,
				     unsigned char metablock[32],
				 	 unsigned int size )
{
	int i;
	unsigned char nonce[12];
	unsigned char mac[16];
	unsigned char ctr[16];
	unsigned char scratchpad[16];
	dsi_context cryptoctx;
	unsigned char *keyX = NULL, *keyY = NULL;

	if (ctx->randomnonce)
	{
		srand( time(0) );

		for(i=0; i<12; i++)
			nonce[i] = rand();
	}
	else
	{
		memcpy(nonce, ctx->nonce, 12);
	}

	if(ctx->keyflags & 1)keyX = ctx->keyX;
	if(ctx->keyflags & 2)keyY = ctx->keyY;
	dsi_init_ccm(&cryptoctx, ctx->key, keyX, keyY, 16, size, 0, nonce);
	dsi_encrypt_ccm(&cryptoctx, buffer, buffer, size, mac);

	memset(scratchpad, 0, 16);
	scratchpad[0] = 0x3A;
	scratchpad[13] = size >> 16;
	scratchpad[14] = size >> 8;
	scratchpad[15] = size >> 0;

	memset(ctr, 0, 16);
	memcpy(ctr+1, nonce, 12);

	dsi_init_ctr(&cryptoctx, ctx->key, keyX, keyY, ctr);
	dsi_crypt_ctr_block(&cryptoctx, scratchpad, metablock+16);
	memcpy(metablock+17, nonce, 12);

	memcpy(metablock, mac, 16);

}
#endif

