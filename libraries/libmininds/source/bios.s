.text
.align 4
.thumb

.global swiDelay
.thumb_func
swiDelay:
swi 0x03
bx lr

#ifdef ARM9
.global swiIntrWait
.thumb_func
swiIntrWait:
swi 0x04
bx lr


.global swiWaitForVBlank
.thumb_func
swiWaitForVBlank:
swi 0x05
bx lr
#endif

