CPU ?= L866
DIRS = driver lib lib/test
GIT_VERSION = $(git describe --dirty --long --tags --always)

all clean modules:
	for d in $(DIRS) ; do make -C $$d $@ CPU=$(CPU) GIT_VERSION=$(GIT_VERSION) ; done
