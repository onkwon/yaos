# Toolchain

CROSS_COMPILE ?= arm-none-eabi
CC := $(CROSS_COMPILE)-gcc
LD := $(CROSS_COMPILE)-ld
OC := $(CROSS_COMPILE)-objcopy
OD := $(CROSS_COMPILE)-objdump

# Common

PROJECT = yaos
VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)
BUILDIR = build

CFLAGS += -fno-builtin -nostdlib -nostartfiles -Wno-main
CFLAGS += -O2 -DVERSION=$(VERSION)
CFLAGS += -Wall -Wunused-parameter -Werror -Wno-main
OCFLAGS =
ODFLAGS = -Dx
LIBS	=

# Configuration

-include .config
include CONFIGURE

# Third party module

include Makefile.3rd

# Build

TARGET    = $(ARCH)
CFLAGS   += -march=$(ARCH) -DMACHINE=$(MACH) -DSOC=$(SOC)
LD_SCRIPT = $(BUILDIR)/generated.lds
LDFLAGS   = -T$(LD_SCRIPT)
ifdef LD_LIBRARY_PATH
	LDFLAGS += -L$(LD_LIBRARY_PATH) -lgcc
endif

SUBDIRS	 = lib kernel fs tasks
INCS	 = -I$(BASEDIR)/include
FILES	 = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call FILES,$d/,$2))

INC_TMP	 = include/asm
INC_ASM	 = $(call FILES,arch/$(ARCH)/include,*.h)
INC_ASM	+= $(call FILES,arch/$(ARCH)/mach-$(MACH)/include,*.h)
INC_ASM	+= $(call FILES,arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include,*.h)

SRCS_ASM = $(call FILES,arch/$(ARCH)/,*.S)
SRCS    += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c $(dir)/**/*.c))
SRCS    += $(wildcard *.c)
SRCS    += $(wildcard arch/$(ARCH)/*.c) $(wildcard arch/$(ARCH)/mach-$(MACH)/*.c)
SRCS    += $(wildcard arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/*.c)

OBJS	 = $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
OBJS	+= $(addprefix $(BUILDIR)/, $(SRCS_ASM:.S=.o))

DEPS	 = $(OBJS:.o=.d)

all: $(BUILDIR) $(BUILDIR)/$(PROJECT).bin $(BUILDIR)/$(PROJECT).hex $(BUILDIR)/$(PROJECT).dump
	@echo "Version      :" $(VERSION)
	@echo "Architecture :" $(ARCH)
	@echo "Vendor       :" $(MACH)
	@echo "SoC          :" $(SOC)
	@echo "Board        :" $(BOARD)
	@echo "\nSection Size(in bytes):"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("%s\t\t %8d\n", $$1, strtonum($$3))}' $(BUILDIR)/$(PROJECT).map

$(BUILDIR): $(INC_TMP) $(LD_SCRIPT)
$(BUILDIR)/%.dump: $(BUILDIR)/%.elf
	$(OD) $(ODFLAGS) $< > $@
$(BUILDIR)/%.hex: $(BUILDIR)/%.elf
	$(OC) $(OCFLAGS) -O ihex $< $@
$(BUILDIR)/%.bin: $(BUILDIR)/%.elf
	$(OC) $(OCFLAGS) -O binary $< $@
$(BUILDIR)/$(PROJECT).elf: $(OBJS) $(THIRD_PARTY_OBJS)
	$(LD) -o $@ $^ -Map $(BUILDIR)/$(PROJECT).map $(LDFLAGS)
$(OBJS): $(BUILDIR)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -MMD -c $< -o $@
$(THIRD_PARTY_OBJS): %.o: %.c
	@mkdir -p $(BUILDIR)/3rd/$(@D)
	$(CC) $(THIRD_PARTY_CFLAGS) $(THIRD_PARTY_INCS) -c $< -o $(addprefix $(BUILDIR)/3rd/,$(notdir $@))
$(LD_SCRIPT): arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) arch/$(ARCH)/common.lds
	@mkdir -p $(@D)
	-cp arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) $@
	$(CC) -E -x c $(CFLAGS) arch/$(ARCH)/common.lds | grep -v '^#' >> $@
.c.o:
	$(CC) $(CFLAGS) $(INCS) $(LIBS) -c $< -o $@

-include $(DEPS)

include/asm: $(INC_ASM)
	@rm -rf include/asm
	-cp -R arch/$(ARCH)/include include/asm
	-cp -R arch/$(ARCH)/mach-$(MACH)/include include/asm/mach
ifdef BOARD
	-cp -R arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include include/asm/mach/board
	-cp -f arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include/pinmap.h include/asm/
	-cp -f arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include/hw.h include/asm/
endif

.PHONY: clean
clean:
	@rm -rf $(BUILDIR)
	@rm -rf include/asm

ifneq ($(MAKECMDGOALS), clean)
	ifneq ($(MAKECMDGOALS), depend)
		ifneq ($(SRCS),)
			-include $(BUILDIR)/$(PROJECT).dep
		endif
	endif
endif

armv7-m4: armv7-m
	@echo "CFLAGS += -mtune=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16" >> .config
	@echo "CFLAGS += -fsingle-precision-constant -Wdouble-promotion" >> .config
armv7-m3: armv7-m
	@echo "CFLAGS += -mtune=cortex-m3" >> .config
armv7-m:
	@echo "ARCH = armv7-m" > .config
	@echo "CFLAGS += -mthumb" >> .config

stm32f4: armv7-m4 stm32
	@echo "SOC = stm32f4" >> .config
stm32f3: armv7-m4 stm32
	@echo "SOC = stm32f3" >> .config
stm32f1: armv7-m3 stm32
	@echo "SOC = stm32f1" >> .config
stm32:
	@echo "MACH = stm32" >> .config

mycortex-stm32f4: stm32f4
	@echo "BOARD = mycortex-stm32f4" >> .config
	@echo "LD_SCRIPT_MACH = stm32f4.lds" >> .config

ust-mpb-stm32f103: stm32f1
	@echo "BOARD = ust-mpb-stm32f103" >> .config
	@echo "LD_SCRIPT_MACH = stm32f1.lds" >> .config

stm32-lcd: stm32f1
	@echo "BOARD = stm32-lcd" >> .config
	@echo "LD_SCRIPT_MACH = stm32f1.lds" >> .config

mango-z1: stm32f1
	@echo "BOARD = mango-z1" >> .config
	@echo "LD_SCRIPT_MACH = boards/mango-z1/memory.lds" >> .config

nrf52: armv7-m4
	@echo "CFLAGS += -DNRF52832_XXAA" >> .config
	@echo "LD_SCRIPT_MACH = nrf52.lds" >> .config
	@echo "MACH = nrf5" >> .config
	@echo "SOC = nrf52" >> .config

stm32f469i-disco: stm32f4
	@echo "BOARD := stm32f469i-disco" >> .config
	@echo "LD_SCRIPT_MACH = boards/$(BOARD)/memory.lds" >> .config
stm32f429i-disco: stm32f4
	@echo "BOARD := stm32f429i-disco" >> .config
	@echo "LD_SCRIPT_MACH = boards/$(BOARD)/memory.lds" >> .config

rpi: rpi-common
	@echo "SOC = bcm2835" >> .config
	@echo "CFLAGS += -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp" >> .config
	#@echo "ARCH = armv6zk" >> .config
rpi2: rpi-common
	@echo "SOC = bcm2836" >> .config
	@echo "CFLAGS += -mtune=cortex-a7 -mfloat-abi=hard -mfpu=vfpv3-d16" >> .config
rpi-common:
	@echo "ARCH = armv7-a" >> .config
	@echo "MACH = rpi" > .config

TTY = /dev/tty.usbmodem141133
.PHONY: burn
burn: $(BUILDIR)/$(PROJECT).bin
	st-flash --reset write $(BUILDIR)/$(PROJECT:%=%.bin) 0x08000000
.PHONY: erase
erase:
	st-flash erase
.PHONY: term
term:
	minicom -D $(TTY)
