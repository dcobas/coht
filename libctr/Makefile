#================================================================
# Makefile to produce timing library
#================================================================

CPU=L866
SOVER=1.1
LDVER=1.1

include /acc/dsc/src/co/Make.auto

DRVR = ../../driver

CFLAGS= -g -Wall -I. -I$(DRVR) -fPIC -DLDVER=$(LDVER) -DSOVER=$(SOVER)

SRCS=libctr.c libctr.h libctrP.h libctrp.c libctrv.c

INSTFILES=libctr.$(CPU).a libctr.h libctrp.$(CPU).so libctrv.$(CPU).so

ACCS=tst

all:libctr.$(CPU).a libctrp.$(CPU).so libctrv.$(CPU).so

libctr.$(CPU).o: $(SRCS)

libctrp.$(CPU).o: libctrp.c libctr_common.c

libctrv.$(CPU).o: libctrv.c libctr_common.c

libctr.$(CPU).a: libctr.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

libctrp.$(CPU).so: libctrp.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

libctrv.$(CPU).so: libctrv.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

clean:
	rm -f libctr.$(CPU).o
	rm -f libctr.$(CPU).a
	rm -f libctrp.$(CPU).o
	rm -f libctrp.$(CPU).so
	rm -f libctrv.$(CPU).o
	rm -f libctrv.$(CPU).so

install:
	@echo -e "\nUse: install-all, install-lib, install-lab, install-oper, install-oplhc\n"

install-all: install-lib install-lab install-oper install-oplhc

install-lib: $(INSTFILES)
	@echo -e "\nInstalling libctr.a in /acc/local/"$(CPU)"/tim"
	dsc_install libctr.$(CPU).a /acc/local/$(CPU)/tim
	rm -rf /acc/local/$(CPU)/tim/libctr.$(LDVER)
	mkdir /acc/local/$(CPU)/tim/libctr.$(LDVER)
	@echo "Installing libctr.a and libctr.h in /acc/local/"$(CPU)"tim/libctr."$(LDVER)
	dsc_install libctr.$(CPU).a /acc/local/$(CPU)/tim/libctr.$(LDVER)
	dsc_install libctr.h /acc/local/$(CPU)/tim/libctr.$(LDVER)
	dsc_install $(DRVR)/*.h /acc/local/$(CPU)/tim/libctr.$(LDVER)
	rm -f /acc/local/$(CPU)/ctr/libctr.h
	@echo "Installing libctr.h in /acc/local/"$(CPU)"/ctr"
	dsc_install libctr.h /acc/local/$(CPU)/ctr

	@echo -e "Done install-lib\n"

install-lab: $(INSTFILES)
	@echo -e "\nInstalling libctrp.so and libctrv.so in lab"
	rm -f /acc/dsc/lab/$(CPU)/ctr/libctrp.so.$(SOVER)
	rm -f /acc/dsc/lab/$(CPU)/ctr/libctrv.so.$(SOVER)
	rm -f /acc/dsc/lab/$(CPU)/ctr/libctrp.so
	rm -f /acc/dsc/lab/$(CPU)/ctr/libctrv.so
	cp libctrp.$(CPU).so /acc/dsc/lab/$(CPU)/ctr/libctrp.so.$(SOVER)
	cp libctrv.$(CPU).so /acc/dsc/lab/$(CPU)/ctr/libctrv.so.$(SOVER)
	cd /acc/dsc/lab/$(CPU)/ctr/; ln -s libctrp.so.$(SOVER) libctrp.so
	cd /acc/dsc/lab/$(CPU)/ctr/; ln -s libctrv.so.$(SOVER) libctrv.so
	@echo -e "Done install-lab\n"

install-oper: $(INSTFILES)
	@echo -e "\nInstalling libctrp.so and libctrv.so in oper"
	rm -f /acc/dsc/oper/$(CPU)/ctr/libctrp.so.$(SOVER)
	rm -f /acc/dsc/oper/$(CPU)/ctr/libctrv.so.$(SOVER)
	rm -f /acc/dsc/oper/$(CPU)/ctr/libctrp.so
	rm -f /acc/dsc/oper/$(CPU)/ctr/libctrv.so
	cp libctrp.$(CPU).so /acc/dsc/oper/$(CPU)/ctr/libctrp.so.$(SOVER)
	cp libctrv.$(CPU).so /acc/dsc/oper/$(CPU)/ctr/libctrv.so.$(SOVER)
	cd /acc/dsc/oper/$(CPU)/ctr/; ln -s libctrp.so.$(SOVER) libctrp.so
	cd /acc/dsc/oper/$(CPU)/ctr/; ln -s libctrv.so.$(SOVER) libctrv.so
	@echo -e "Done install-oper\n"

install-oplhc: $(INSTFILES)
	@echo -e "\nInstalling libctrp.so and libctrv.so in oplhc"
	rm -f /acc/dsc/oplhc/$(CPU)/ctr/libctrp.so.$(SOVER)
	rm -f /acc/dsc/oplhc/$(CPU)/ctr/libctrv.so.$(SOVER)
	rm -f /acc/dsc/oplhc/$(CPU)/ctr/libctrp.so
	rm -f /acc/dsc/oplhc/$(CPU)/ctr/libctrv.so
	cp libctrp.$(CPU).so /acc/dsc/oplhc/$(CPU)/ctr/libctrp.so.$(SOVER)
	cp libctrv.$(CPU).so /acc/dsc/oplhc/$(CPU)/ctr/libctrv.so.$(SOVER)
	cd /acc/dsc/oplhc/$(CPU)/ctr/; ln -s libctrp.so.$(SOVER) libctrp.so
	cd /acc/dsc/oplhc/$(CPU)/ctr/; ln -s libctrv.so.$(SOVER) libctrv.so
	@echo -e "Done install-oplhc\n"

