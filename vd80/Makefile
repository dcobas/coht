.PHONY: clean all

CPU ?= L865
DIRS = driver lib

all clean:
	for dir in $(DIRS) ; do $(MAKE) -C $$dir $@ CPU=$(CPU) ; done
