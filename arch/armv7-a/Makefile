SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC      = -I$(BASEDIR)/include -I./include
LIBS    +=
CFLAGS  += $(INC) $(LIBS)

all: subs $(OBJS) lds
	$(LD) -o mach-$(MACH).o -r mach-$(MACH)/*.o

subs:
	@$(MAKE) --print-directory -C mach-$(MACH)

lds:
	-cp mach-$(MACH)/$(SOC).lds ld.script

.s.o:
.S.o:
	$(CC) $(CFLAGS) -c $<

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

.PHONY: include
include:
	-cp -R mach-$(MACH)/include include/mach-$(MACH)
	-cp -R mach-$(MACH)/include/pinmap.h include/pinmap.h
	-cp -R mach-$(MACH)/include/usart.h include/usart.h

clean:
	@rm -f $(OBJS) .depend
	@for i in $(SUBDIRS); do $(MAKE) clean -C $$i || exit $?; done
	@rm -f $(wildcard *.o)
	@rm -f ld.script
	@$(MAKE) clean -C mach-$(MACH)
	@rm -rf include/mach-$(MACH)
	@rm -f include/pinmap.h
	@rm -f include/usart.h

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
