#ifndef _H_TWLSDMMC_
#define _H_TWLSDMMC_

#ifndef ARM7
#ifndef ARM9
#error "Define either ARM7 or ARM9."
#endif
#endif

#ifdef ARM7
#include "twlwifi7.h"
#endif
#ifdef ARM9
#include "twlwifi9.h"
#endif

#endif

