include $(BASEDIR)/CONFIGURE

SRCS_ASM = $(wildcard *.S)
SRCS     = fs.c ramfs.c
ifdef CONFIG_FS
	SRCS += embedfs.c
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
	@rm -f $(OBJS) .depend $(wildcard *.o)

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
