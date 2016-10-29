#ifdef USELIBNDS
#include <nds.h>
#else
#include <mininds/mininds.h>
#endif
#include "twlwifi.h"

#ifdef ARM9
#include "wifisdioipc.h"

static wifisdioipc_cmd wifisdio_cmd;
extern char strbuf[];
#endif

vu32 twlwifi_arm7ready_flg = 0;

void twlwifi_sendmsg(wifisdioipc_cmd *cmd);

#ifndef NOIPC
void twlwifi_arm7readyhandler(u32 data, void* userdata)
{
	//printf("got mmc chan data %x\n", data);
	if(data==0x594452)
	{
		twlwifi_arm7ready_flg = 1;//"RDY"
	}
}

void twlwifi_miniarm7readyhandler(u32 data)
{
	twlwifi_arm7readyhandler(data, 0);
}

u32 twlwifi_arm7ready()
{
	return twlwifi_arm7ready_flg;
}
#endif

void twlwifi_init()
{
	#ifndef NOIPC
	#ifdef USELIBNDS
	fifoSetValue32Handler(TWLWIFI_FIFOCHAN, twlwifi_arm7readyhandler, 0);
	#else
	mininds_setfifochanhandler(TWLWIFI_FIFOCHAN, twlwifi_miniarm7readyhandler);
	#endif
	while(!twlwifi_arm7ready_flg);
	wifisdio_cmd.cmdtype = 2;
	twlwifi_sendmsg(&wifisdio_cmd);
	#endif
}

void twlwifi_sendmsg(wifisdioipc_cmd *cmd)
{
	#ifndef NOIPC
	wifisdio_cmd.busy = 1;
	DC_FlushRange(cmd, sizeof(wifisdioipc_cmd));
	#ifdef USELIBNDS
	fifoSendAddress(TWLWIFI_FIFOCHAN, cmd);
	#else
	mininds_sendfifodata(TWLWIFI_FIFOCHAN, (u32)cmd);
	#endif
	while(1)
	{
		DC_InvalidateRange(cmd, sizeof(wifisdioipc_cmd));
		if(cmd->busy==0)break;
	}
	#endif
}

