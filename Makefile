VERSION = $(shell git describe --all | sed 's/^.*\///').$(shell git describe --abbrev=4 --dirty --always)
BASEDIR = $(shell pwd)
export BASEDIR

ARCH = cortex-m3
MCU  = stm32f103

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OC = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

CFLAGS  = -Wall -O2 $(INCS) -mcpu=$(ARCH) -mthumb -fno-builtin #-fno-common
CFLAGS += -DVERSION=\"$(VERSION)\" -DHSE=8000000
ifeq ($(RELEASE),)
CFLAGS += -g -DDEBUG -O0
endif
LDFLAGS = -nostartfiles -Tibox.lds
OCFLAGS = -O binary
ODFLAGS = -Dsx
export CC OC OD CFLAGS OCFLAGS ODFLAGS

SRCS_ASM = $(wildcard *.S)
SRCS = $(shell ls | grep .*.c$$ | sed 's/main.c//')
OBJS = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)
TARGET_SRCS = main.c
TARGET_OBJS = $(TARGET_SRCS:%.c=%.o)
TARGET = $(TARGET_SRCS:%.c=%)

INCS = -I./include
LIBS = 
export INCS LIBS
SUBDIRS = drivers lib tasks

all: $(TARGET)
	@echo "Section Size(in bytes):"
	@awk '/^.text/ || /^.data/ || /^.bss/ {printf("%s\t\t %8d\n", $$1, strtonum($$3))}' $(TARGET).map

.SECONDEXPANSION:
$(TARGET): $$@.o $(OBJS) subs
	$(LD) $(LDFLAGS) -o $@ $< $(OBJS) $(patsubst %, %/*.o, $(SUBDIRS)) -Map $@.map
	$(OC) $(OCFLAGS) $@ $@.bin
	$(OD) $(ODFLAGS) $@ > $@.dump

subs:
	@for i in $(SUBDIRS); do $(MAKE) --print-directory -C $$i || exit $?; done

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

depend dep:
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

clean:
	@rm -f $(OBJS) $(TARGET_OBJS) $(TARGET) .depend
	@rm -f $(TARGET:%=%.map)
	@for i in $(SUBDIRS); do $(MAKE) clean -C $$i || exit $?; done
	@rm -f $(TARGET:%=%.bin)
	@rm -f $(TARGET:%=%.dump)

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif

burn:
	tools/stm32flash/stm32flash -w $(TARGET:%=%.bin) -v -g 0x0 /dev/ttyUSB0
dev:
	tools/stm32flash/stm32flash /dev/ttyUSB0
