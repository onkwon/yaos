# Machine dependant

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Common

PROJECT = yaos
VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)

# Options

SUBDIRS = lib arch kernel fs drivers tasks
CFLAGS += -Wall -O2 -fno-builtin -nostdlib -nostartfiles -DVERSION=$(VERSION)
OCFLAGS = -O binary
ODFLAGS = -Dx
INC	= -I./include
LIBS	=

# Configuration

include CONFIGURE
-include .config

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
	ifdef CONFIG_SYSCALL_THREAD
		CFLAGS += -DCONFIG_SYSCALL_THREAD
	endif
endif
ifdef CONFIG_FS
	CFLAGS += -DCONFIG_FS
endif
ifdef CONFIG_TIMER
	CFLAGS += -DCONFIG_TIMER
endif
ifdef CONFIG_FLOAT
	CFLAGS += -DCONFIG_FLOAT
endif
ifdef CONFIG_CPU_LOAD
	CFLAGS += -DCONFIG_CPU_LOAD
endif
ifdef CONFIG_SOFTIRQ_THREAD
	CFLAGS += -DCONFIG_SOFTIRQ_THREAD
endif
ifdef CONFIG_DEBUG
	CFLAGS += -g -DCONFIG_DEBUG #-O0
	ifdef CONFIG_DEBUG_TASK
		CFLAGS += -DCONFIG_DEBUG_TASK
	endif
	ifdef CONFIG_DEBUG_SYSCALL
		CFLAGS += -DCONFIG_DEBUG_SYSCALL
	endif
endif
ifdef CONFIG_SLEEP_LONG
	CFLAGS += -DCONFIG_SLEEP_LONG
endif
ifdef CONFIG_SLEEP_DEEP
	CFLAGS += -DCONFIG_SLEEP_DEEP
endif
ifdef CONFIG_IRQ_HIERARCHY
	CFLAGS += -DCONFIG_IRQ_HIERARCHY
endif

# Module

ifdef MODULE_UGFX
	GFXLIB = $(BASEDIR)/ugfx
	export GFXLIB
	include $(GFXLIB)/boards/base/Olimex-STM32-LCD/board.mk
	include $(GFXLIB)/gfx.mk

	INC += $(foreach d, $(GFXINC), -I$d)
	SRCS +=$(GFXSRC)
endif

# Build

TARGET  = $(ARCH)
ifeq ($(SOC),bcm2835)
	TARGET  = armv7-a
endif
CFLAGS += -march=$(ARCH) -DMACHINE=$(MACH) -DSOC=$(SOC)
LDFLAGS = -Tarch/$(TARGET)/generated.lds -L$(LD_LIBRARY_PATH) -lgcc

SRCS_ASM = $(wildcard *.S)
SRCS    += $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

export BASEDIR
export TARGET MACH SOC BOARD LD_SCRIPT
export CC LD OC OD CFLAGS LDFLAGS OCFLAGS ODFLAGS
export INC LIBS

all: include common
	@echo "\nArchitecture :" $(ARCH)
	@echo "Vendor       :" $(MACH)
	@echo "SoC          :" $(SOC)
	@echo "Board        :" $(BOARD)
	@echo "\nSection Size(in bytes):"
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
	-cp -R drivers/include include/drivers
	-cp -R lib/include include/lib
	-cp -R fs/include include/fs

.c.o:
	$(CC) $(CFLAGS) $(INC) $(LIBS) -c $< -o $@

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

mycortex-stm32f4: stm32f4
	@echo "BOARD = mycortex-stm32f4" >> .config
	@echo "LD_SCRIPT = stm32f4.lds" >> .config

ust-mpb-stm32f103: stm32f1
	@echo "BOARD = ust-mpb-stm32f103" >> .config
	@echo "LD_SCRIPT = stm32f1.lds" >> .config

stm32-lcd: stm32f1
	@echo "BOARD = stm32-lcd" >> .config
	@echo "LD_SCRIPT = stm32f1.lds" >> .config

mango-z1: stm32f1
	@echo "BOARD = mango-z1" >> .config
	@echo "LD_SCRIPT = boards/mango-z1/memory.lds" >> .config

nrf52: armv7-m4
	@echo "CFLAGS += -DNRF52832_XXAA" >> .config
	@echo "LD_SCRIPT = nrf52.lds" >> .config
	@echo "MACH = nrf5" >> .config
	@echo "SOC = nrf52" >> .config

stm32f4: armv7-m4 stm32
	@echo "SOC = stm32f4" >> .config
stm32f3: armv7-m4 stm32
	@echo "SOC = stm32f3" >> .config
stm32f1: armv7-m3 stm32
	@echo "SOC = stm32f1" >> .config
stm32:
	@echo "MACH = stm32" >> .config

rpi: rpi-common
	@echo "ARCH = armv6zk" >> .config
	@echo "SOC = bcm2835" >> .config
	@echo "CFLAGS += -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp" >> .config
rpi2: rpi-common
	@echo "ARCH = armv7-a" >> .config
	@echo "SOC = bcm2836" >> .config
	@echo "CFLAGS += -mtune=cortex-a7 -mfloat-abi=hard -mfpu=vfpv3-d16" >> .config
rpi-common:
	@echo "MACH = rpi" > .config

armv7-m4: armv7-m
	@echo "CFLAGS += -mtune=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16" >> .config
armv7-m3: armv7-m
	@echo "CFLAGS += -mtune=cortex-m3 -mthumb" >> .config
armv7-m:
	@echo "ARCH = armv7-m" > .config

TTY = /dev/tty.SLAB_USBtoUART
.PHONY: burn
burn:
	st-flash --reset write $(PROJECT:%=%.bin) 0x08000000
.PHONY: erase
erase:
	st-flash erase
.PHONY: term
term:
	minicom -D $(TTY)
