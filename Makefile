# Machine dependant

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Configuration

include CONFIGURE
ifdef CONFIG_SMP
CFLAGS += -DCONFIG_SMP
endif
ifdef CONFIG_REALTIME
CFLAGS += -DCONFIG_REALTIME
endif
ifdef CONFIG_PAGING
CFLAGS += -DCONFIG_PAGING
endif
ifdef CONFIG_SYSCALL
CFLAGS += -DCONFIG_SYSCALL
endif
ifdef CONFIG_FS
CFLAGS += -DCONFIG_FS
endif
ifdef CONFIG_TIMER
CFLAGS += -DCONFIG_TIMER
endif

# Common 

PROJECT = yaos

-include .config
TARGET  = $(ARCH)
ifeq ($(SOC),bcm2835)
TARGET  = armv7-a
endif
export TARGET MACH SOC

VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)
export BASEDIR

CFLAGS += -march=$(ARCH)
CFLAGS += -Wall -O2 -fno-builtin -nostdlib -nostartfiles
CFLAGS += -DVERSION=$(VERSION) -DMACHINE=$(MACH) -DSOC=$(SOC)
ifdef CONFIG_DEBUG
CFLAGS += -g -DCONFIG_DEBUG #-O0
endif
LDFLAGS = -Tarch/$(TARGET)/ld.script -L$(LD_LIBRARY_PATH) -lgcc
OCFLAGS = -O binary
ODFLAGS = -DxS
export CC LD OC OD CFLAGS LDFLAGS OCFLAGS ODFLAGS

SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC  = -I./include
LIBS = 
export INC LIBS

SUBDIRS = lib arch kernel fs drivers tasks

all: include common
	@echo "Section Size(in bytes):"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("%s\t\t %8d\n", $$1, strtonum($$3))}' $(PROJECT).map

common: $(OBJS) subdirs
	$(LD) -o $(PROJECT).elf $(OBJS) $(patsubst %, %/*.o, $(SUBDIRS)) -Map $(PROJECT).map $(LDFLAGS)
	$(OC) $(OCFLAGS) $(PROJECT).elf $(PROJECT).bin
	$(OD) $(ODFLAGS) $(PROJECT).elf > $(PROJECT).dump

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) --print-directory -C $@

.PHONY: include
include:
	@$(MAKE) include --print-directory -C arch/$(TARGET)
	-cp -R arch/$(TARGET)/include include/asm
	-cp -R drivers/include include/driver
	-cp -R lib/include include/lib
	-cp -R fs/include include/fs

.c.o:
	$(CC) $(CFLAGS) $(INC) $(LIBS) -c $<

.PHONY: depend dep
depend dep:
	$(CC) $(CFLAGS) -MM $(SRCS) > .depend

.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) clean -C $$i || exit $?; done
	@rm -f $(OBJS) $(PROJECT:%=%.elf) .depend
	@rm -f $(PROJECT:%=%.map)
	@rm -f $(PROJECT:%=%.bin)
	@rm -f $(PROJECT:%=%.dump)
	@rm -rf include/asm
	@rm -rf include/driver
	@rm -rf include/lib
	@rm -rf include/fs

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif

stm32f4:
	@echo "ARCH = armv7-m\nMACH = stm32\nSOC = stm32f4\nCFLAGS += -mtune=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16" > .config
stm32f1:
	@echo "ARCH = armv7-m\nMACH = stm32\nSOC = stm32f1\nCFLAGS += -mtune=cortex-m3 -mthumb" > .config
rpi:
	@echo "ARCH = armv6zk\nMACH = rpi\nSOC = bcm2835\nCFLAGS += -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp" > .config
rpi2:
	@echo "ARCH = armv7-a\nMACH = rpi\nSOC = bcm2836\nCFLAGS += -mtune=cortex-a7 -mfloat-abi=hard -mfpu=vfpv3-d16" > .config

TTY = /dev/tty.SLAB_USBtoUART
.PHONY: burn
burn:
	st-flash --reset write $(PROJECT:%=%.bin) 0x08000000
.PHONY: term
term:
	minicom -D $(TTY)
