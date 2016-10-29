#include "mininds.h"

void mininds_sysinit()
{
	register int i;
	//stop timers and dma
	for (i=0; i<4; i++) 
	{
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}


	//clear video display registers
	dmaFillWords(0, (void*)0x04000000, 0x56);  
	dmaFillWords(0, (void*)0x04001008, 0x56);  
	videoSetModeSub(0);

	//vramDefault();

	VRAM_E_CR = 0;
	VRAM_F_CR = 0;
	VRAM_G_CR = 0;
	VRAM_H_CR = 0;
	VRAM_I_CR = 0;

	irqInit();
	irqEnable(IRQ_VBLANK);

	//fifoInit();
	mininds_initfifo();
}

