#include "mininds.h"
#include <string.h>

#define MININDS_MAXFIFOQUENESZ 1

static mininds_fifochanhandler fifochan_handlers[MININDS_MAXFIFOCHANS];
//static u32 mininds_fifochan_buf[MININDS_MAXFIFOCHANS*MININDS_MAXFIFOQUENESZ];
//static u32 mininds_fifochan_bufnum[MININDS_MAXFIFOCHANS];

void mininds_fifoirq();
void mininds_fifichanreset(u32 addr);

void mininds_fifochan_debug(u32 data)
{
	//printf("libmininds fifochandebug: recieved data on chan without handler: %x\n", data);
}

void mininds_initfifo()
{
	int myval = 0, count = 0, i;
	for(i=0; i<MININDS_MAXFIFOCHANS; i++)fifochan_handlers[i] = mininds_fifochan_debug;
	//memset(mininds_fifochan_buf, 0, MININDS_MAXFIFOCHANS*MININDS_MAXFIFOQUENESZ*4);
	//memset(mininds_fifochan_bufnum, 0, MININDS_MAXFIFOCHANS*4);
	mininds_setfifochanhandler(0, mininds_fifichanreset);
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;
	irqSet(IRQ_FIFO_NOT_EMPTY, mininds_fifoirq);
	irqEnable(IRQ_FIFO_NOT_EMPTY);


	while(count<3)
	{
		#ifdef ARM9
		IPC_SendSync(myval);
		myval++;
		while(IPC_GetSync()!=myval);
		#endif
		#ifdef ARM7
		while(IPC_GetSync()!=myval);
		myval++;
		IPC_SendSync(myval);
		#endif
		count++;
	}
}

void mininds_setfifochanhandler(int chan, mininds_fifochanhandler cb)
{
	int i;
	fifochan_handlers[chan] = cb;
	if(!cb)fifochan_handlers[chan] = mininds_fifochan_debug;
	/*if(mininds_fifochan_bufnum[chan])
	{
		for(i=0; i<mininds_fifochan_bufnum[chan]; i++)
		{
			mininds_fifochan_bufnum[chan]--;
			cb(mininds_fifochan_buf[chan]);
		}
	}*/
}

void mininds_sendfifodata(int chan, u32 data)
{
	u32 msg = (chan & 0xf) | (data<<4);

	while(REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL);
	while(REG_IPC_FIFO_CR & IPC_FIFO_ERROR);

	REG_IPC_FIFO_TX = msg;
}

void mininds_fifoirq()
{
	u32 msg;
	int chan;
	u32 data;
	mininds_fifochanhandler cb;
	while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
	{
		msg = REG_IPC_FIFO_RX;
		chan = msg & 0xf;
		data = msg>>4;
		cb = fifochan_handlers[chan];
		if(cb)cb(data);
		/*if(!cb && mininds_fifochan_bufnum[chan]<MININDS_MAXFIFOQUENESZ)
		{
			mininds_fifochan_bufnum[chan]++;
			mininds_fifochan_buf[chan] = data;
		}*/
	}
}

#ifdef ARM9
void resetARM7(u32 addr)
#else
void resetARM9(u32 addr)
#endif
{
	mininds_sendfifodata(0, addr);
}

void mininds_fifichanreset(u32 addr)
{
	void(*cb)() = (void*)addr;
	cb();
}

