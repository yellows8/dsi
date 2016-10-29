#ifndef _H_MMCIPC
#define _H_MMCIPC

#define TWLESCRYPT_FIFOCHAN FIFO_USER_07

typedef struct _escryptipc_cmd
{
	u32 busy;
	u32 cmdtype;
	int retval;
	u32 numargs;
	u32 args[12];
} escryptipc_cmd;

#endif
