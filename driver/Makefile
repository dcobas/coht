include ../Makefile.base
include ../Makefile.specific

all:
	@./compiledrvr

linux:
	@./compiledrvr linux

lynx:
	@./compiledrvr lynx

# CERN delivery
include ../deliver.mk
deliver:
	$(Q)$(MAKE) _deliver $(filter $(strip $(ACCS)),$(MAKECMDGOALS)) CPU=L865
	$(Q)$(MAKE) _deliver $(filter $(strip $(ACCS)),$(MAKECMDGOALS)) CPU=ppc4