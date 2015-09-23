include $(BASEDIR)/CONFIGURE

SRCS_ASM = $(wildcard *.S)
SRCS     = sched.c fair.c
ifdef CONFIG_REALTIME
	SRCS += rt.c
endif
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

all: $(OBJS)

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

clean:
	@rm -f $(OBJS) .depend

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
