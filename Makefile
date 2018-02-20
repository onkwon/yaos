# Toolchain

CROSS_COMPILE ?= arm-none-eabi
CC := $(CROSS_COMPILE)-gcc
LD := $(CROSS_COMPILE)-ld
AR := $(CROSS_COMPILE)-ar
OC := $(CROSS_COMPILE)-objcopy
OD := $(CROSS_COMPILE)-objdump

# Common

PROJECT = yaos
VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)
BUILDIR = build

CFLAGS += -fno-builtin -nostdlib -nostartfiles
CFLAGS += -O2 -DVERSION=$(VERSION)
CFLAGS += -Wall -Wunused-parameter -Werror -Wno-main
ARFLAGS = rcs
OCFLAGS =
ODFLAGS = -Dsx
LIBS   +=

# Configuration

-include .config
include CONFIGURE

# Third party module

include Makefile.3rd

# Build

Q	 := @
TARGET    = $(ARCH)
CFLAGS   += -march=$(ARCH) -DMACHINE=$(MACH) -D$(SOC)
LD_SCRIPT = $(BUILDIR)/generated.ld
LDFLAGS   = -T$(LD_SCRIPT)
ifdef LD_LIBRARY_PATH
	LDFLAGS += -L$(LD_LIBRARY_PATH) -lgcc
endif

SUBDIRS	 = lib kernel fs tasks
INCS	+= -I$(BASEDIR)/include
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
THIRD_PARTY_OBJS = $(addprefix $(BUILDIR)/3rd/, $(THIRD_PARTY_SRCS:.c=.o))
OUTPUTS	 = $(addprefix $(BUILDIR)/$(PROJECT)., a bin hex dump)

DEPS	 = $(OBJS:.o=.d) $(THIRD_PARTY_OBJS:.o=.d)

all: $(BUILDIR) $(OUTPUTS)
	@printf "\n  Version      : $(VERSION)\n"
	@printf "  Architecture : $(ARCH)\n"
	@printf "  Vendor       : $(MACH)\n"
	@printf "  SoC          : $(SOC)\n"
	@printf "  Board        : $(BOARD)\n\n"
	@printf "  Section Size(in bytes):\n"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("  %s\t\t %8d\n", $$1, strtonum($$3))}' $(BUILDIR)/$(PROJECT).map
	@wc -c $(BUILDIR)/$(PROJECT).bin | awk '{printf("  .bin\t\t %8d\n", $$1)}'
	@printf "\n  $(shell sha256sum $(BUILDIR)/$(PROJECT).bin)\n"

$(BUILDIR)/%.dump: $(BUILDIR)/%.elf
	@printf "  OD       $@\n"
	$(Q)$(OD) $(ODFLAGS) $< > $@
$(BUILDIR)/%.hex: $(BUILDIR)/%.elf
	@printf "  OC       $@\n"
	$(Q)$(OC) $(OCFLAGS) -O ihex $< $@
$(BUILDIR)/%.bin: $(BUILDIR)/%.elf
	@printf "  OC       $@\n"
	$(Q)$(OC) $(OCFLAGS) -O binary $< $@
$(BUILDIR)/$(PROJECT).elf: $(OBJS) $(THIRD_PARTY_OBJS)
	@printf "  LD       $@\n"
	$(Q)$(LD) -o $@ $^ $(LIBS) -Map $(BUILDIR)/$(PROJECT).map $(LDFLAGS)
$(BUILDIR)/$(PROJECT).a: $(OBJS) $(THIRD_PARTY_OBJS)
	@printf "  AR       $@\n"
	$(Q)$(AR) $(ARFLAGS) -o $@ $^
$(OBJS): $(BUILDIR)/%.o: %.c Makefile CONFIGURE .config
	@printf "  CC       $<\n"
	@mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(INCS) -MMD -c $< -o $@
$(THIRD_PARTY_OBJS): $(BUILDIR)/3rd/%.o: %.c Makefile Makefile.3rd CONFIGURE .config
	@printf "  CC       $(<F)\n"
	@mkdir -p $(@D)
	$(Q)$(CC) $(THIRD_PARTY_CFLAGS) $(THIRD_PARTY_INCS) -c $< -o $@
$(LD_SCRIPT): arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) arch/$(ARCH)/common.ld .config
	@printf "  GEN      $@\n"
	@mkdir -p $(@D)
	$(Q)-cp arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) $@
	$(Q)$(CC) -E -x c $(CFLAGS) arch/$(ARCH)/common.ld | grep -v '^#' >> $@
.c.o:
	@printf "  CC       $(<F)\n"
	$(Q)$(CC) $(CFLAGS) $(INCS) -c $< -o $@
$(BUILDIR): $(INC_TMP) $(LD_SCRIPT)

-include $(DEPS)

include/asm: $(INC_ASM) .config
	@printf "  COPY     $@\n"
	@rm -rf include/asm
	$(Q)-cp -R arch/$(ARCH)/include include/asm
	$(Q)-cp -R arch/$(ARCH)/mach-$(MACH)/include include/asm/mach
	$(Q)-cp include/asm/mach/$(SOC).h include/asm/mach/reg.h
ifdef BOARD
	$(Q)-cp -R arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include include/asm/mach/board
	$(Q)-cp -f arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include/pinmap.h include/asm/
	$(Q)-cp -f arch/$(ARCH)/mach-$(MACH)/boards/$(BOARD)/include/hw.h include/asm/
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
	@echo "LD_SCRIPT_MACH = stm32f4.ld" >> .config

ust-mpb-stm32f103: stm32f1
	@echo "BOARD = ust-mpb-stm32f103" >> .config
	@echo "LD_SCRIPT_MACH = stm32f1.ld" >> .config

stm32-lcd: stm32f1
	@echo "BOARD = stm32-lcd" >> .config
	@echo "LD_SCRIPT_MACH = stm32f1.ld" >> .config

mango-z1: stm32f1
	@echo "BOARD = mango-z1" >> .config
	@echo "LD_SCRIPT_MACH = boards/mango-z1/memory.ld" >> .config

nrf52: armv7-m4
	@echo "CFLAGS += -DNRF52832_XXAA" >> .config
	@echo "LD_SCRIPT_MACH = nrf52.ld" >> .config
	@echo "MACH = nrf5" >> .config
	@echo "SOC = nrf52" >> .config

stm32f469i-disco: stm32f4
	@echo "BOARD = stm32f469i-disco" >> .config
	@echo "LD_SCRIPT_MACH = boards/stm32f469i-disco/memory.ld" >> .config
stm32f429i-disco: stm32f4
	@echo "BOARD = stm32f429i-disco" >> .config
	@echo "LD_SCRIPT_MACH = boards/stm32f429i-disco/memory.ld" >> .config

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
