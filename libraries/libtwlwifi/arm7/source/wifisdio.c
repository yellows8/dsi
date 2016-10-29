#ifdef USELIBNDS
#include <nds.h>
#else
#include <mininds/mininds.h>
#define NULL 0
#endif
#include <stdio.h>
#include "wifisdio.h"

#define WIFMMC_BASE	0x04004a00
#define DISPSTAT_CHK_VBLANK   (1<<0)
//#define REG_DISPSTAT      *(vu16*)0x04000004

#define wifisdio_send_command twlwifi_wifisdio_send_command
#define wifisdio_send_acmd41 twlwifi_wifisdio_send_acmd41
#define wifisdio_clkdelay0 twlwifi_wifisdio_clkdelay0
#define wifisdio_clkdelay1 twlwifi_wifisdio_clkdelay1
#define wifisdio_clkdelay twlwifi_wifisdio_clkdelay


void SendFIFOString(char *str);

#define IOHook_LogStr SendFIFOString
//#define TWLDEBUG

//char logstr[256];
void IOHook_LogHex(void* buf, int sz);

u32 twlwifi_cid[4];
extern char *fifo_string;

static int wifisdio_timeout = 0;
static int wifisdio_gotcmd8reply = 0;
static int wifisdio_sdhc = 0;
static int wifisdio_ismem = 0, wifisdio_isio = 0, wifisdio_numiofuncs = 0;

void wifisdio_initdelaytimers();

inline u16 wifisdio_read16(u16 reg) {
	return *(vu16*)(WIFISDIO_BASE + reg);
}

inline void wifisdio_write16(u16 reg, u16 val) {
	*(vu16*)(WIFISDIO_BASE + reg) = val;
}

inline void wifisdio_mask16(u16 reg, u16 clear, u16 set) {
	u16 val = wifisdio_read16(reg);
	val &= ~clear;
	val |= set;
	wifisdio_write16(reg, val);
}

int wifisdio_send_command(u16 cmd, u16 arg0, u16 arg1) {
	u16 is_stat0, was_stat0;
	u16 is_stat1, was_stat1;

	wifisdio_mask16(REG_SDCLKCTL, 0x100, 0x0);

	wifisdio_write16(REG_SDCMDARG0, arg0);
	wifisdio_write16(REG_SDCMDARG1, arg1);

	was_stat1 = wifisdio_read16(REG_SDSTATUS1);

	while(wifisdio_read16(REG_SDSTATUS1) & 0x4000);

	is_stat1 = wifisdio_read16(REG_SDSTATUS1);

	wifisdio_mask16(REG_SDSTATUS1, 0x807F, 0);
	wifisdio_mask16(REG_SDSTATUS0, 0x0001, 0);
	wifisdio_mask16(REG_SDSTATUS0, 0x0004, 0);
	wifisdio_mask16(REG_SDSTATUS1, 0x100, 0);
	wifisdio_mask16(REG_SDSTATUS1, 0x200, 0);
	wifisdio_mask16(REG_SDSTOP, 1, 0);
	wifisdio_mask16(0x22, 0x807F, 0);
	wifisdio_mask16(0x20, 0x4, 0);
	wifisdio_timeout = 0;

	if(cmd==17 || cmd==18) {
		if((wifisdio_read16(0x100) & 2)==0)wifisdio_mask16(0x22, 0x100, 0);
	}
	if(cmd==24 || cmd==25) {
		if((wifisdio_read16(0x100) & 2)==0)wifisdio_mask16(0x22, 0x200, 0);
	}

	wifisdio_mask16(REG_SDSTOP, 1, 0);
	wifisdio_write16(REG_SDCMD, cmd);

	if (cmd != 0) {
		was_stat0 = wifisdio_read16(REG_SDSTATUS0);

		if (cmd != 0x5016) {
			while((wifisdio_read16(REG_SDSTATUS0) & 5) == 0) {
				if(wifisdio_read16(REG_SDSTATUS1) & 0x40) {
					wifisdio_mask16(REG_SDSTATUS1, 0x40, 0);
					wifisdio_timeout = 1;
					break;
				}
			}
		}

		is_stat0 = wifisdio_read16(REG_SDSTATUS0);
		wifisdio_mask16(REG_SDSTATUS0, 5, 0);
	}

	if(wifisdio_timeout)SendFIFOString("TIMEOUT!");
	sprintf(fifo_string, "CMD STATUS: %04x%04x", wifisdio_read16(REG_SDSTATUS1), wifisdio_read16(REG_SDSTATUS0));
	SendFIFOString(NULL);

	// irq mask?
	/*if(cmd!=17 && cmd!=18 && cmd!=24 && cmd!=25)*/wifisdio_mask16(0x22, 0, 0x807F);
	wifisdio_mask16(REG_SDCLKCTL, 0, 0x100);

	return 0;
}

void wifisdio_controller_init() {
	// Reset
	wifisdio_write16(0x100, 0x0402);
	wifisdio_write16(0x100, 0x0000);
	wifisdio_write16(0x104, 0x0000);
	wifisdio_write16(0x108, 0x0001);

	// InitIP
	wifisdio_mask16(REG_SDRESET, 0x0003, 0x0000);
	wifisdio_mask16(REG_SDRESET, 0x0000, 0x0003);
	wifisdio_mask16(REG_SDSTOP, 0x0001, 0x0000);

	// Reset
	wifisdio_mask16(REG_SDOPT, 0x0005, 0x0000);

	// EnableInfo
	wifisdio_mask16(REG_SDSTATUS0, 0x0018, 0x0000);
	wifisdio_mask16(0x20, 0x0018, 0x0000); // ??
	wifisdio_mask16(0x20, 0x0000, 0x0018);
}

u8 wifisdio_send_cmdrd_iodirect(u32 func, u32 regadr)
{
	u32 resp = 0;
	u32 arg = ((func & 7)<<27) | ((regadr&0x1ffff)<<9);
	wifisdio_send_command(0x434, (u16)arg, (arg>>16));

	resp = wifisdio_read16(REG_SDRESP0) | (wifisdio_read16(REG_SDRESP1)<<16);
	sprintf(fifo_string, "CMD52RD RESP: %x", resp);
	SendFIFOString(NULL);
	return (u8)resp;
}

void wifisdio_send_cmdwr_iodirect(u32 func, u32 regadr, u8 value)
{
	u32 resp = 0;
	u32 arg = (1<<31) | ((func & 7)<<27) | ((regadr&0x1ffff)<<9) | (value);
	wifisdio_send_command(0x434, (u16)arg, (arg>>16));

	while( (REG_DISPSTAT & DISPSTAT_CHK_VBLANK) == 0 );
	while( (REG_DISPSTAT & DISPSTAT_CHK_VBLANK) != 0 );
	
	resp = wifisdio_read16(REG_SDRESP0) | (wifisdio_read16(REG_SDRESP1)<<16);
	sprintf(fifo_string, "CMD52WR RESP: %x", resp);
	SendFIFOString(NULL);
}

void wifisdio_send_cmd_iodirect_set32(u32 func, u32 regadr, u8 val)
{
	u32 tmp = wifisdio_send_cmdrd_iodirect(func, regadr);
	wifisdio_send_cmdwr_iodirect(func, regadr, val | tmp);
}

void wifisdio_send_cmd_iodirect_clear32(u32 func, u32 regadr, u8 val)
{
	u32 tmp = wifisdio_send_cmdrd_iodirect(func, regadr);
	wifisdio_send_cmdwr_iodirect(func, regadr, ~val & tmp);
}

int wifisdio_ath_init() {
	u16 sdaddr;
	u32 resp0;
	u32 resp1;
	u32 resp2;
	u32 resp3;
	u32 resp4;
	u32 resp5;
	u32 resp6;
	u32 resp7;
	u32 ocr, ccs, hcs, response, io_ocr;
	int vblank_counter = 0;

	/*wifisdio_write16(REG_SDCLKCTL, 0x20);
	wifisdio_write16(REG_SDOPT, 0x40D0); // XXX: document me!
	wifisdio_write16(REG_SDDEVICE, 0x401);

	wifisdio_mask16(REG_SDCLKCTL, 0, 0x100);
	wifisdio_write16(REG_SDCLKCTL, wifisdio_read16(REG_SDCLKCTL));
	wifisdio_mask16(REG_SDCLKCTL, 0x100, 0);
	wifisdio_mask16(REG_SDCLKCTL, 0, 0x200);
	wifisdio_mask16(REG_SDCLKCTL, 0, 0x100);*/
	
	SendFIFOString("Sending reset cmds...");

	wifisdio_send_cmdrd_iodirect(0, 0);

	return 0;
}

