DIRS = ipack carriers devices

all modules install modules_install:
	for d in $(DIRS); do $(MAKE) -C $$d $@ || exit 1; done
clean:
	rm -rf Module.symvers modules.order
	for d in $(DIRS); do $(MAKE) -C $$d $@ || exit 1; done

