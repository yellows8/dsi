#ifndef _H_WIFISDIOIPC
#define _H_WIFISDIOIPC

#define TWLWIFI_FIFOCHAN FIFO_USER_08

typedef struct _wifisdioipc_cmd
{
	u32 busy;
	u32 cmdtype;
	u32 sector;
	u32 total_sectors;
	u32 *cryptbuf;
	u32 *buffer;
	int raw;
	int rw;
} wifisdioipc_cmd;

#endif
