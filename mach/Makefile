SRCS_ASM = $(wildcard $(TARGET)/*.S)
SRCS     = $(wildcard $(TARGET)/*.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

all:
	@$(MAKE) --print-directory -C $(TARGET)
	$(LD) -o $(TARGET).o -r $(OBJS)

clean:
	@$(MAKE) clean -C $(TARGET)
	@rm -f $(TARGET).o $(wildcard *.o)
