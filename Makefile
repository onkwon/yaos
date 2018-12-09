# Configuration

-include .config
include CONFIGURE

# Third party module

include Makefile.3rd

# Toolchain

CROSS_COMPILE ?= arm-none-eabi
CC := $(CROSS_COMPILE)-gcc
LD := $(CROSS_COMPILE)-ld
SZ := $(CROSS_COMPILE)-size
AR := $(CROSS_COMPILE)-ar
OC := $(CROSS_COMPILE)-objcopy
OD := $(CROSS_COMPILE)-objdump

# Common

VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR := $(shell pwd)
BUILDIR := build

DEFS += -DVERSION=$(VERSION) -DMACHINE=$(MACH) -D$(SOC) -DLOADADDR=$(LOADADDR)
LIBS += #-lc -lm -lnosys

ARFLAGS = rcs
OCFLAGS =
ODFLAGS = -Dsx

CFLAGS += -std=gnu99 -O2 \
	  -fno-builtin -ffunction-sections -fdata-sections \
	  -Wl,--gc-sections #-flto
CFLAGS += -pedantic -W -Wall -Wunused-parameter -Wno-main -Wextra #-Werror -Wpointer-arith
CFLAGS += -Wformat-nonliteral -Wcast-align -Wpointer-arith -Wbad-function-cast \
	  -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
	  -Winline -Wundef -Wnested-externs -Wshadow -Wconversion \
	  -Wwrite-strings -Wno-conversion -Wstrict-aliasing -Wcast-qual
ifndef NDEBUG
	CFLAGS += -g -Og
endif
ifeq ($(CONFIG_FLOAT),y)
CFLAGS += -u _printf_float
endif
CFLAGS += -march=$(ARCH)

LD_SCRIPT = $(BUILDIR)/generated.ld

LDFLAGS += -specs=nano.specs #-specs=nosys.specs
LDFLAGS += -nostartfiles #-nostdlib
LDFLAGS += -T$(LD_SCRIPT)
ifdef LD_LIBRARY_PATH
	LDFLAGS += -L$(LD_LIBRARY_PATH) -lgcc
endif
LDFLAGS += $(LIBS)

# Build

TARGET   = yaos
SUBDIRS	 = lib kernel tasks
INCS	+= -I$(BASEDIR)/include
FILES	 = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call FILES,$d/,$2))

ARCH_INCDIR = include/arch
INC_ARCH = $(call FILES,arch/$(ARCH)/include,*.h) \
	   $(call FILES,arch/$(ARCH)/mach-$(MACH)/include,*.h) \
	   $(call FILES,arch/$(ARCH)/mach-$(MACH)/board/$(BOARD)/include,*.h)

SRCS_ASM = $(call FILES,arch/$(ARCH)/,*.S)
SRCS    += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c $(dir)/**/*.c)) \
	   $(wildcard arch/$(ARCH)/*.c) $(wildcard arch/$(ARCH)/mach-$(MACH)/*.c) \
	   $(wildcard arch/$(ARCH)/mach-$(MACH)/board/$(BOARD)/*.c) \
	   $(wildcard *.c)

OBJS	 = $(addprefix $(BUILDIR)/, $(SRCS:.c=.o)) \
	   $(addprefix $(BUILDIR)/, $(SRCS_ASM:.S=.o))
THIRD_PARTY_OBJS = $(addprefix $(BUILDIR)/3rd/, $(THIRD_PARTY_SRCS:.c=.o))
OUTPUTS	 = $(addprefix $(BUILDIR)/$(TARGET)., a bin hex dump sha256 img)

DEPS	 = $(OBJS:.o=.d) $(THIRD_PARTY_OBJS:.o=.d)

Q ?= @
ifneq ($(Q),@)
	override undefine Q
endif

all: $(BUILDIR) $(OUTPUTS)
	@printf "\n  Version      : $(VERSION)\n"
	@printf "  Architecture : $(ARCH)\n"
	@printf "  Vendor       : $(MACH)\n"
	@printf "  SoC          : $(SOC)\n"
	@printf "  Board        : $(BOARD)\n\n"
	@printf "  Section Size(in bytes):\n"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("  %s\t\t %8d\n", $$1, strtonum($$3))}' $(BUILDIR)/$(TARGET).map
	@wc -c $(BUILDIR)/$(TARGET).bin | awk '{printf("\n  .bin\t\t %8d\n", $$1)}'
	@wc -c $(BUILDIR)/$(TARGET).img | awk '{printf("  .img\t\t %8d\n", $$1)}'
	@printf "\n  sha256: $(shell cat $(BUILDIR)/$(TARGET).sha256)\n"

.PHONY: doc
doc: $(ARCH_INCDIR)
	$(Q)cd doc && LD_LIBRARY_PATH=/Library/Developer/CommandLineTools/usr/lib $(MAKE) html

.PRECIOUS: $(BUILDIR)/%.signature $(BUILDIR)/%.enc
$(BUILDIR)/%.img: $(BUILDIR)/%.enc $(BUILDIR)/%.signature
	@printf "  IMAGE    $@\n"
	$(Q)printf "DEADC0DE DEADC1DE DEADC2DE" | xxd -r -p > $@
	$(Q)wc -c < $< | awk '{printf("%08x", $$1)}' | tools/endian.sh | xxd -r -p >> $@
	$(Q)cat examples/aes128.iv | xxd -r -p >> $@
	@#$(Q)echo $(shell sha256sum $<) | awk '{printf("%s\n", $$1)}' | xxd -r -p >> $@
	$(Q)cat $(@D)/$(*F).signature | xxd -r -p >> $@
	$(Q)cat $< >> $@
	@# Initial BootOpt for bootloader. Upload this at LOADADDR - sector size.
	$(Q)echo $(UPLOAD) | sed -e "s/^0x//" | tools/endian.sh | xxd -r -p > $(@D)/bootopt.bin
	$(Q)wc -c < $< | awk '{printf("%08x", $$1)}' | tools/endian.sh | xxd -r -p >> $(@D)/bootopt.bin
	$(Q)cat $(@D)/$(*F).signature | xxd -r -p >> $(@D)/bootopt.bin
	$(Q)cat examples/aes128.iv | xxd -r -p >> $(@D)/bootopt.bin
$(BUILDIR)/%.signature: $(BUILDIR)/%.enc
	@printf "  SIGN     $@\n"
	$(Q)$(shell openssl dgst -sha256 -sign examples/secp256r1-key.pem $< > $(@D)/$(*F).der)
	$(Q)xxd -p -c 128 $(@D)/$(*F).der | tools/der2bin.sh > $@
$(BUILDIR)/%.enc: $(BUILDIR)/%.bin
	@printf "  ENCRYPT  $@\n"
	$(Q)openssl enc -aes-128-ctr -K $(shell cat examples/aes128.key) -iv $(shell cat examples/aes128.iv) -in $< -out $@ #-base64
$(BUILDIR)/%.sha256: $(BUILDIR)/%.bin
	@printf "  HASH     $@\n"
	$(Q)echo $(shell sha256sum $<) > $@
$(BUILDIR)/%.dump: $(BUILDIR)/%.elf
	@printf "  OD       $@\n"
	$(Q)$(OD) $(ODFLAGS) $< > $@
$(BUILDIR)/%.hex: $(BUILDIR)/%.elf
	@printf "  OC       $@\n"
	$(Q)$(OC) $(OCFLAGS) -O ihex $< $@
$(BUILDIR)/%.bin: $(BUILDIR)/%.elf
	@printf "  OC       $@\n"
	$(Q)$(OC) $(OCFLAGS) -O binary $< $@
$(BUILDIR)/$(TARGET).elf: $(OBJS) $(THIRD_PARTY_OBJS)
	@printf "  LD       $@\n"
	@#$(Q)$(LD) -o $@ $^ -Map $(BUILDIR)/$(TARGET).map $(LDFLAGS)
	$(Q)$(CC) $(CFLAGS) $(INCS) $(DEFS) -o $@ $^ -Wl,-Map,$(BUILDIR)/$(TARGET).map $(LDFLAGS)
	@#$(SZ) $@
$(BUILDIR)/$(TARGET).a: $(OBJS) $(THIRD_PARTY_OBJS)
	@printf "  AR       $@\n"
	$(Q)$(AR) $(ARFLAGS) -o $@ $^
$(OBJS): $(BUILDIR)/%.o: %.c Makefile CONFIGURE .config $(LD_SCRIPT)
	@printf "  CC       $<\n"
	@mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) $(INCS) $(DEFS) -MMD -c $< -o $@
$(THIRD_PARTY_OBJS): $(BUILDIR)/3rd/%.o: %.c Makefile Makefile.3rd CONFIGURE .config
	@printf "  CC       $(<F)\n"
	@mkdir -p $(@D)
	$(Q)$(CC) $(THIRD_PARTY_CFLAGS) $(THIRD_PARTY_INCS) $(DEFS) -c $< -o $@
$(LD_SCRIPT): arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) arch/$(ARCH)/common.ld .config
	@printf "  GEN      $@\n"
	@mkdir -p $(@D)
	$(Q)-cp arch/$(ARCH)/mach-$(MACH)/$(LD_SCRIPT_MACH) $@
	$(Q)$(CC) -E -x c $(CFLAGS) $(DEFS) arch/$(ARCH)/common.ld | grep -v '^#' >> $@
.c.o:
	@printf "  CC       $(<F)\n"
	$(Q)$(CC) $(CFLAGS) $(INCS) $(DEFS) -c $< -o $@
$(BUILDIR): $(ARCH_INCDIR) $(LD_SCRIPT)

-include $(DEPS)

$(ARCH_INCDIR): $(INC_ARCH) .config
	@printf "  COPY     $@\n"
	@rm -rf $(ARCH_INCDIR)
	$(Q)-cp -R arch/$(ARCH)/include $(ARCH_INCDIR)
	$(Q)-cp -R arch/$(ARCH)/mach-$(MACH)/include $(ARCH_INCDIR)/mach
	$(Q)-cp $(ARCH_INCDIR)/mach/$(SOC).h $(ARCH_INCDIR)/mach/regs.h
ifdef BOARD
	$(Q)-cp -R arch/$(ARCH)/mach-$(MACH)/board/$(BOARD)/include $(ARCH_INCDIR)/mach/board
endif

.PHONY: clean
clean: cleantest
	@rm -rf $(BUILDIR)
	@rm -rf $(ARCH_INCDIR)

ifneq ($(MAKECMDGOALS), clean)
	ifneq ($(MAKECMDGOALS), depend)
		ifneq ($(SRCS),)
			-include $(BUILDIR)/$(TARGET).dep
		endif
	endif
endif

# .config

include Makefile.arch

# Tools

TTY = /dev/tty.usbmodem141133
.PHONY: burn flash
flash burn: $(BUILDIR)/$(TARGET).bin
	st-flash --reset write $(BUILDIR)/$(TARGET:%=%.bin) $(LOADADDR)
.PHONY: upload
upload: $(BUILDIR)/$(TARGET).img
	st-flash --reset write $(BUILDIR)/$(TARGET:%=%.img) $(UPLOAD)
.PHONY: erase
erase:
	st-flash erase
.PHONY: term
term:
	minicom -D $(TTY)

# Unit test

include Makefile.tst
