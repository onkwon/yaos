SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INCS    += -I$(BASEDIR)/include
LIBS    += 
CFLAGS  += $(INCS) $(LIBS)

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
