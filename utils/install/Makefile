include /acc/src/dsc/co/Make.auto

OBJECTDIR=object
CFLAGS += -I. -I/acc/local/$(CPU)/include -I/usr/include/libxml2 -I /acc/src/dsc/drivers/coht/include -I ../include
LDLIBS=-lxml2 -lz
ifeq ($(CPU),ppc4)
	LDLIBS += -liberty
	LDFLAGS += -L /acc/local/ppc4/lib
	LDFLAGS += -L /acc/sys/623/cdk/linux-elf-ppc/lib/
	CPU_OBJECTS = extra_for_lynx.$(CPU).o
else
	CPU_OBJECTS = inst-linux.$(CPU).o
endif

all: lib inst

lib: libutils.$(CPU).a
inst: instprog.$(CPU)

instprog.$(CPU): instprog.$(CPU).o libutils.$(CPU).a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

libutils.$(CPU).a: $(CPU_OBJECTS) libinst.$(CPU).o inst-utils.$(CPU).o
	ar r $@ $^
	mkdir -p $(OBJECTDIR)
	ln -sf $(PWD)/$@ $(OBJECTDIR)

clean:
	rm -f *.o *.a *.L865 *.ppc4
	rm -rf $(OBJECTDIR)
