CPU ?= L865

DIRS= \
	ad9516 \
	driver \
	lib \
	test

all clean modules:
	for d in $(DIRS) ; do $(MAKE) -C $$d CPU=$(CPU) $@ || exit 1 ; done
