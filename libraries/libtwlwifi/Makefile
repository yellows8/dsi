#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)

PARAMS	:=	
ifeq ($(USELIBNDS),1)
	PARAMS	+=	USELIBNDS=1
endif
ifeq ($(strip $(USELIBNDS)),)
	PARAMS	+=	USELIBMININDS=1
endif
ifeq ($(NOIPC),1)
	PARAMS	+=	NOIPC=1
endif
ifeq ($(ARM7DISK),1)
	PARAMS	+=	ARM7DISK=1
endif
ifeq ($(DEBUG),1)
	PARAMS	+=	DEBUG=1
endif

.PHONY: arm7/$(TARGET).elf arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all:	release

#---------------------------------------------------------------------------------
release	:
	$(MAKE) -C arm7 $(PARAMS)
	$(MAKE) -C arm9 $(PARAMS)
	@cp arm7/lib/$(TARGET)7.a $(LIBNDS)/lib
	@cp arm9/lib/$(TARGET)9.a $(LIBNDS)/lib
	@mkdir -p $(LIBNDS)/include/twlwifi
	@cp include/twlwifi.h $(LIBNDS)/include/twlwifi
	@cp arm7/include/twlwifi7.h $(LIBNDS)/include/twlwifi
	@cp arm9/include/twlwifi9.h $(LIBNDS)/include/twlwifi

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
