SRCS_ASM = $(wildcard $(MACH)/*.S)
SRCS     = $(wildcard $(MACH)/*.c)
OBJS     = $(SRCS:%.c=%.o) $(SRCS_ASM:%.S=%.o)

all:
	@$(MAKE) --print-directory -C $(MACH)
	$(LD) -o $(MACH).o -r $(OBJS)

clean:
	@$(MAKE) clean -C $(MACH)
	@rm -f $(MACH).o
