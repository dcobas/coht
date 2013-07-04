CPU ?= L866
DIRS = driver lib lib/test

all clean modules:
	for d in $(DIRS) ; do make -C $$d $@ CPU=$(CPU) ; done
