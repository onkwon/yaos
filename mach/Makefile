all:
	@$(MAKE) --print-directory -C $(TARGET)
	$(LD) -o $(TARGET).o -r $(TARGET)/*.o

clean:
	@$(MAKE) clean -C $(TARGET)
	@rm -f $(TARGET).o $(wildcard *.o)
