SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC      = -I$(BASEDIR)/include -I./include
LIBS    +=
CFLAGS  += $(INC) $(LIBS)

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

all: subs $(OBJS) lds
	$(LD) -o mach-$(MACH).o -r mach-$(MACH)/*.o

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

lds:
	-cp mach-$(MACH)/$(LD_SCRIPT) generated.lds
	$(CC) -E -x c $(CFLAGS) common.lds | grep -v '^#' >> generated.lds

subs:
	@$(MAKE) --print-directory -C mach-$(MACH)

.PHONY: include
include:
	-cp -R mach-$(MACH)/include include/mach-$(MACH)
	-cp mach-$(MACH)/include/pinmap.h include/pinmap.h
	-cp mach-$(MACH)/include/uart.h include/uart.h
	-cp mach-$(MACH)/include/timer.h include/timer.h
	-cp mach-$(MACH)/include/hw.h include/hw.h
	-cp mach-$(MACH)/include/adc.h include/adc.h
ifdef BOARD
	-cp -f mach-$(MACH)/boards/$(BOARD)/include/pinmap.h include/pinmap.h
	-cp -f mach-$(MACH)/boards/$(BOARD)/include/hw.h include/hw.h
endif

clean:
	@rm -f $(OBJS) .depend
	@rm -f generated.lds
	@$(MAKE) clean -C mach-$(MACH)
	@rm -f mach-$(MACH).o
	@rm -rf include/mach-$(MACH)
	@rm -f include/pinmap.h
	@rm -f include/uart.h
	@rm -f include/timer.h
	@rm -f include/hw.h
	@rm -f include/adc.h

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
