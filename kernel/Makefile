include $(BASEDIR)/CONFIGURE

SRCS_ASM = $(wildcard *.S)
SRCS     = task.c sched.c cfs.c mm.c jiffies.c time.c waitqueue.c softirq.c
ifdef CONFIG_REALTIME
	SRCS += rts.c
endif
ifdef CONFIG_PAGING
	SRCS += buddy.c
endif
ifdef CONFIG_SYSCALL
	SRCS += syscall.c module.c
endif
ifdef CONFIG_FS
	SRCS += fs.c
endif
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC     += -I$(BASEDIR)/include
LIBS    += 
CFLAGS  += $(INC) $(LIBS)

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
