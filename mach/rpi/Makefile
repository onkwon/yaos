SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC      = -I$(BASEDIR)/include -I./include
LIBS    +=
CFLAGS  += $(INC) $(LIBS)

all: $(OBJS)

.s.o:
.S.o:
	$(CC) $(CFLAGS) -c $<

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
