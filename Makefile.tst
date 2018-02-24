ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
    CLEANUP = del /F /Q
    MKDIR = mkdir
  else # in a bash-like shell, like msys
    CLEANUP = rm -f
    MKDIR = mkdir -p
  endif
    TARGET_EXTENSION=.exe
else
    CLEANUP = rm -f
    MKDIR = mkdir -p
    TARGET_EXTENSION=out
endif

.PHONY: testclean
.PHONY: test

PATHU = tools/Unity/src/
PATHS = ./
PATHT = test/
PATHB = build/
PATHD = build/depends/
PATHO = build/objs/
PATHR = build/results/

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

TSTSRC = $(wildcard $(PATHT)*.c)

COMPILE=gcc -c
LINK=gcc
DEPEND=gcc -MM -MG -MF
TCFLAGS =-Iinclude -I$(PATHU) -I$(PATHS) -DTEST $(DEFS)

RESULTS = $(patsubst $(PATHT)Test%.c,$(PATHR)Test%.txt,$(TSTSRC) )
ORGSRC = $(patsubst $(PATHT)Test%.c,%.c,$(TSTSRC))
ALLSRC = $(foreach w,$(ORGSRC),$(shell find . -name $(w)))
ALLOBJ = $(foreach s,$(ALLSRC),$(PATHO)$(notdir $(s:.c=.o)))

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

define create_rule =
$(eval test_o := $(PATHO)$(notdir $(1:.c=.o)))
$(test_o) : $(1)
	$(Q)$(COMPILE) $(TCFLAGS) $(1) -o $(test_o)
endef

test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:"
	@echo "$(PASSED)"
	@echo "DONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	@echo "  TESTING  " $@ $<
	@-./$< > $@ 2>&1

$(PATHB)Test%.$(TARGET_EXTENSION): $(PATHO)Test%.o $(PATHO)%.o $(PATHO)unity.o #$(PATHD)Test%.d
	$(Q)$(LINK) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c
	$(Q)$(COMPILE) $(TCFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c
	$(Q)$(COMPILE) $(TCFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h
	$(Q)$(COMPILE) $(TCFLAGS) $< -o $@

$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHB):
	$(Q)$(MKDIR) $(PATHB)

$(PATHD):
	$(Q)$(MKDIR) $(PATHD)

$(PATHO):
	$(Q)$(MKDIR) $(PATHO)

$(PATHR):
	$(Q)$(MKDIR) $(PATHR)

$(foreach test_s,$(ALLSRC),$(eval $(call create_rule,$(test_s))))

testclean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt

.PRECIOUS: $(PATHB)Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
