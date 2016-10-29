#ifndef _H_MININDS7_
#define _H_MININDS7_

#ifndef ARM7
#ifndef ARM9
#error "Define either ARM7 or ARM9."
#endif
#endif

#include <nds.h>

#ifdef ARM7
#include "mininds7.h"
#endif
#ifdef ARM9
#include "mininds9.h"
#endif

#define MININDS_MAXFIFOCHANS 16
typedef void (*mininds_fifochanhandler)(u32 data);
void mininds_initfifo();
void mininds_setfifochanhandler(int chan, mininds_fifochanhandler cb);
void mininds_sendfifodata(int chan, u32 data);

#endif

