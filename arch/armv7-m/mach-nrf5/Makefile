SRCS_ASM = $(wildcard *.S)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

INC      = -I$(BASEDIR)/include -I./include
INC	+= -I$(NRF_SDK)/components/device
INC	+= -I$(NRF_SDK)/components/toolchain
INC	+= -I$(NRF_SDK)/components/toolchain/cmsis/include
INC	+= -I$(NRF_SDK)/components/drivers_nrf/hal
INC	+= -I$(NRF_SDK)/components/drivers_nrf/delay
INC	+= -I$(NRF_SDK)/components/drivers_nrf/nrf_soc_nosd
INC	+= -I$(NRF_SDK)/components/libraries/util
INC	+= -I$(NRF_SDK)/components/libraries/uart

THIRD_PARTY = $(NRF_SDK)/components/toolchain/system_$(SOC).c

LIBS    +=
CFLAGS  += $(INC) $(LIBS)

.SUFFIXES: .c.o
.SUFFIXES: .s.o
.SUFFIXES: .S.o

all: board $(OBJS) $(SOC).o
ifdef BOARD
	$(LD) -o $(BOARD).o -r boards/$(BOARD)/*.o
endif

board:
ifdef BOARD
	@$(MAKE) --print-directory -C boards/$(BOARD)
endif

$(SOC).o: $(THIRD_PARTY)
	$(CC) $(CFLAGS) $(INC) $(LIBS) -c $< -o $@

depend dep:
	echo $(CC)
	$(CC) $(CFLAGS) -MM $(SRCS) $(TARGET_SRCS) > .depend

clean:
	@rm -f $(OBJS) .depend 
	@rm -f $(SOC).o
ifdef BOARD
	@rm -f $(BOARD).o
	@$(MAKE) clean -C boards/$(BOARD)
endif

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
ifneq ($(MAKECMDGOALS), dep)
ifneq ($(SRCS),)
-include .depend
endif
endif
endif
endif
