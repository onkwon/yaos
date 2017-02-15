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
	-cp mach-$(MACH)/$(SOC).lds ld.script

subs:
	@$(MAKE) --print-directory -C mach-$(MACH)

.PHONY: include
include:
	-cp -R mach-$(MACH)/include include/mach-$(MACH)
	-cp mach-$(MACH)/include/pinmap.h include/pinmap.h
	-cp mach-$(MACH)/include/usart.h include/usart.h
	-cp mach-$(MACH)/include/timer.h include/timer.h
ifdef BOARD
	-cp -f mach-$(MACH)/boards/$(BOARD)/include/pinmap.h include/pinmap.h
endif

clean:
	@rm -f $(OBJS) .depend
	@rm -f ld.script
	@$(MAKE) clean -C mach-$(MACH)
	@rm -f mach-$(MACH).o
	@rm -rf include/mach-$(MACH)
	@rm -f include/pinmap.h
	@rm -f include/usart.h
	@rm -f include/timer.h

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
