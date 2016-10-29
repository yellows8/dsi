#ifdef USELIBNDS
#include <nds.h>
#else
#include <mininds/mininds.h>
#endif
#include "twlwifi.h"
#include "wifisdioipc.h"
#include "wifisdio.h"

static u32 twlwifi_arm9ready = 0;

#ifndef NOIPC
void wifisdiohandler(void * address, void * userdata)
{
	wifisdioipc_cmd *cmd = (wifisdioipc_cmd*)address;
	switch(cmd->cmdtype)
	{
		case 0:
			/*if(!cmd->rw)twlsdmmc_readsector(cmd->buffer, cmd->sector, cmd->total_sectors, cmd->raw);
			#ifdef ENABLEWR
			if(cmd->rw)twlsdmmc_writesector(cmd->buffer, cmd->sector, cmd->total_sectors, cmd->raw);
			#endif*/
		break;

		case 2:
			twlwifi_arm9ready = 1;
		break;
	}
	
	cmd->busy = 0;
}

void mini_wifisdiohandler(u32 data)
{
	wifisdiohandler((void*)data, 0);
}
#endif

int twlwifi_init()
{
	int retval = 0;

	SendFIFOString("work. now.");
	//wifisdio_initirq();
	//wifisdio_controller_init();
	SendFIFOString("cmds init.....");
	retval = wifisdio_ath_init();
	SendFIFOString("alive");
	#ifndef NOIPC
	twlwifi_arm9ready = 0;
	#ifdef USELIBNDS
	fifoSetAddressHandler(TWLWIFI_FIFOCHAN, wifisdiohandler, 0);
	#else
	mininds_setfifochanhandler(TWLWIFI_FIFOCHAN, mini_wifisdiohandler);
	#endif

	if(retval)return retval;
	while(!twlwifi_arm9ready)
	{
		#ifdef USELIBNDS
		fifoSendValue32(TWLWIFI_FIFOCHAN,0x594452);
		#else
		mininds_sendfifodata(TWLWIFI_FIFOCHAN,0x594452);//"RDY"
		#endif
		swiWaitForVBlank();
	}

	#endif
	return retval;
}

