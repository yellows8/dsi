#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)


.PHONY: arm7/$(TARGET).elf arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all:	release

#---------------------------------------------------------------------------------
release	:
	$(MAKE) -C arm7
	$(MAKE) -C arm9
	@cp arm7/lib/$(TARGET)7.a $(LIBNDS)/lib
	@cp arm9/lib/$(TARGET)9.a $(LIBNDS)/lib
	@cp arm7/startup/mininds_dsi_arm7_crt0.s $(DEVKITARM)/arm-eabi/lib
	@cp arm7/startup/mininds_dsi_arm7_crt0.o $(DEVKITARM)/arm-eabi/lib
	@cp arm7/startup/mininds_dsi_arm7.specs $(DEVKITARM)/arm-eabi/lib
	@cp arm7/startup/dsi_arm7.specs $(DEVKITARM)/arm-eabi/lib
	@cp arm7/startup/dsi_arm7.ld $(DEVKITARM)/arm-eabi/lib
	@cp arm9/startup/mininds_dsi_arm9_crt0.s $(DEVKITARM)/arm-eabi/lib
	@cp arm9/startup/mininds_dsi_arm9_crt0.o $(DEVKITARM)/arm-eabi/lib
	@cp arm9/startup/mininds_dsi_arm9.specs $(DEVKITARM)/arm-eabi/lib
	@mkdir -p $(LIBNDS)/include/mininds
	@cp include/mininds.h $(LIBNDS)/include/mininds
	@cp arm7/include/mininds7.h $(LIBNDS)/include/mininds
	@cp arm9/include/mininds9.h $(LIBNDS)/include/mininds

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9
