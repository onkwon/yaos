# Machine dependant

MACH    = stm32
ARCH    = cortex-m3
export MACH

CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OC      = arm-none-eabi-objcopy
OD      = arm-none-eabi-objdump

CFLAGS  = -mcpu=$(ARCH) -mthumb

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

TARGET  = yaos

VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)
export BASEDIR

CFLAGS += -Wall -O2 -fno-builtin
CFLAGS += -DVERSION=\"$(VERSION)\" -DMACHINE=\"$(MACH)\"
ifdef CONFIG_DEBUG
	CFLAGS += -g -DCONFIG_DEBUG -O0
endif
LDFLAGS = -nostartfiles -Tmach/$(MACH)/$(TARGET).lds
OCFLAGS = -O binary
ODFLAGS = -Dsx
export CC LD OC OD CFLAGS LDFLAGS OCFLAGS ODFLAGS

SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC  = -I./include
LIBS = 
export INC LIBS

SUBDIRS = lib mach kernel fs drivers tasks

all: include $(TARGET)
	@echo "Section Size(in bytes):"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("%s\t\t %8d\n", $$1, strtonum($$3))}' $(TARGET).map

$(TARGET): $(OBJS) subdirs
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(patsubst %, %/*.o, $(SUBDIRS)) -Map $@.map
	$(OC) $(OCFLAGS) $@ $@.bin
	$(OD) $(ODFLAGS) $@ > $@.dump

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) --print-directory -C $@

.PHONY: include
include:
	-cp -R mach/$(MACH)/include include/asm
	-cp -R drivers/include include/driver
	-cp -R lib/include include/lib
	-cp -R fs/include include/fs

.c.o:
	$(CC) -c $< $(CFLAGS) $(INC) $(LIBS)

.SUFFIXES: .s.o
.SUFFIXES: .S.o

.PHONY: depend dep
depend dep:
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) clean -C $$i || exit $?; done
	@rm -f $(OBJS) $(TARGET_OBJS) $(TARGET) .depend
	@rm -f $(TARGET:%=%.map)
	@rm -f $(TARGET:%=%.bin)
	@rm -f $(TARGET:%=%.dump)
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

.PHONY: burn
burn:
	tools/stm32flash/stm32flash -w $(TARGET:%=%.bin) -v -g 0x0 /dev/ttyUSB0
.PHONY: dev
dev:
	tools/stm32flash/stm32flash /dev/ttyUSB0
