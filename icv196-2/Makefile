.PHONY: all clean modules install modules_install
.PHONY: gitmodules prereq prereq_install prereq_install_warn

DIRS = driver lib test

all clean modules:
	for d in $(DIRS); do $(MAKE) -C $$d $@ || exit 1; done
